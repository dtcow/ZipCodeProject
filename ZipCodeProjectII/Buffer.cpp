/** @file Buffer.cpp
 * @brief CSV and length-indicated buffering, parsing, and record validation.
 */
#include "Buffer.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <iomanip>
#include <limits>
#include <sstream>

bool Buffer::readFields(std::istream& source, std::vector<std::string>& fields) {
    fields.clear();
    std::string field;

    bool quoted = false, closedQuote = false, started = false;
    char c;

    while (source.get(c)) {
        started = true;
        if (quoted) {
            if (c == '"') {
                if (source.peek() == '"') { source.get(c); field += '"'; }
                else { quoted = false; closedQuote = true; }
            } else { field += c; }
        } else if (c == ',') {
            fields.push_back(field); field.clear(); closedQuote = false;
        } else if (c == '\n' || c == '\r') {
            if (c == '\r' && source.peek() == '\n') source.get(c);
            fields.push_back(field); ++recordNumber; return true;
        } else if (c == '"' && field.empty() && !closedQuote) {
            quoted = true;
        } else {
            if (closedQuote || c == '"')
                throw std::runtime_error("Invalid CSV quoting near record " + std::to_string(recordNumber + 1));
            field += c;
        }
    }

    if (source.bad()) throw std::runtime_error("Failed while reading CSV");
    if (quoted) throw std::runtime_error("Unterminated quoted CSV field");
    if (!started) return false;
    
    fields.push_back(field); ++recordNumber; return true;
}

Buffer::Buffer(const std::string& filename) : input(filename, std::ios::binary) {
    if (!input) throw std::runtime_error("Cannot open CSV: " + filename);

    //remove an optional UTF-8 BOM before reading the logical header.
    if (input.peek() == 0xEF) {
        char bom[3]; input.read(bom, 3);
        if (!input || static_cast<unsigned char>(bom[1]) != 0xBB ||
            static_cast<unsigned char>(bom[2]) != 0xBF)
            throw std::runtime_error("Invalid UTF-8 BOM");
    }

    std::vector<std::string> header;

    if (!readFields(input, header) || header.size() != 6)
        throw std::runtime_error("Expected six CSV header columns");

    for (auto& name : header) {
        name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) { return std::isspace(c); }), name.end());
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    const std::vector<std::string> expected{"zipcode", "placename", "state", "county", "lat", "long"};

    /// Map each field name to its column so reordering a CSV does not change results.
    for (std::size_t i = 0; i < expected.size(); ++i) {
        auto position = std::find(header.begin(), header.end(), expected[i]);
        if (position == header.end())
            throw std::runtime_error("Missing CSV column: " + expected[i]);
        columns[i] = position - header.begin();
    }
}

bool Buffer::next(ZipCodeRecord& record) {
    std::vector<std::string> fields;

    while (true) {
        if (lengthIndicated) {
            if (input.peek() == std::char_traits<char>::eof()) {
                if (input.eof()) return false;
                throw std::runtime_error("Failed while reading indexed data");
            }
            const auto size = HeaderBuffer::readSize(input, HeaderBuffer::recordSizeWidth);
            /// Reject a payload length that extends beyond the data file.
            const auto remaining = dataEnd - input.tellg();
            if (!input || remaining < 0 || size == 0 ||
                size > static_cast<unsigned long long>(remaining) ||
                size > static_cast<unsigned long long>(std::numeric_limits<std::streamsize>::max()))
                throw std::runtime_error("Invalid length-indicated record size");

            std::string payload(static_cast<std::size_t>(size), '\0');
            if (!input.read(payload.data(), static_cast<std::streamsize>(size)))
                throw std::runtime_error("Truncated length-indicated record");
            std::istringstream row(std::move(payload));
            if (!readFields(row, fields) || row.peek() != std::char_traits<char>::eof())
                throw std::runtime_error("Expected one CSV row in the record payload");
        } else if (!readFields(input, fields)) {
            return false;
        }
        if (fields.size() == 1 && fields[0].empty() && !lengthIndicated) continue;

        try {
            if (fields.size() != 6) throw std::runtime_error("Expected six fields");
            record.zip = fields[columns[0]];
            if (record.zip.empty() || record.zip.size() > 5 ||
                !std::all_of(record.zip.begin(), record.zip.end(), [](unsigned char c) { return std::isdigit(c); }))
                throw std::runtime_error("Invalid ZIP code");
            record.zip.insert(0, 5 - record.zip.size(), '0');
            record.place = fields[columns[1]]; record.state = fields[columns[2]];
            record.county = fields[columns[3]]; ///< Retain county for the complete lookup output.
            if (record.state.size() != 2) throw std::runtime_error("Invalid state ID");
            std::size_t used;
            record.latitude = std::stod(fields[columns[4]], &used);
            if (used != fields[columns[4]].size() || !std::isfinite(record.latitude) ||
                record.latitude < -90 || record.latitude > 90)
                throw std::runtime_error("Invalid latitude");
            record.longitude = std::stod(fields[columns[5]], &used);
            if (used != fields[columns[5]].size() || !std::isfinite(record.longitude) ||
                record.longitude < -180 || record.longitude > 180)
                throw std::runtime_error("Invalid longitude");
        } catch (const std::exception& e) {
            throw std::runtime_error("ZIP record " + std::to_string(recordNumber) + ": " + e.what());
        }
        return true;
    }
}

/** @brief Quote and escape one text field for a comma-separated payload. */
static void writeField(std::ostream& output, const std::string& field) {
    output << '"';
    for (char c : field) {
        if (c == '"') output << '"';
        output << c;
    }
    output << '"';
}

Buffer::Buffer(const std::string& filename, DataHeader& header)
    : input(filename, std::ios::binary), lengthIndicated(true) {
    if (!input) throw std::runtime_error("Cannot open indexed data: " + filename);
    header = HeaderBuffer::read(input);
    input.seekg(0, std::ios::end);
    dataEnd = input.tellg();
    input.seekg(header.headerSize);
    if (!input) throw std::runtime_error("Cannot locate indexed records");
    columns = {0, 1, 2, 3, 4, 5};
}

void Buffer::writeRecord(std::ostream& output, const ZipCodeRecord& record) {
    std::ostringstream row;
    row << record.zip << ',';
    writeField(row, record.place);
    row << ',';
    writeField(row, record.state);
    row << ',';
    writeField(row, record.county);
    row << ',' << std::setprecision(std::numeric_limits<double>::max_digits10)
        << record.latitude << ',' << record.longitude;
    const std::string payload = row.str();
    HeaderBuffer::writeSize(output, payload.size(), HeaderBuffer::recordSizeWidth);
    output << payload;
    if (!output) throw std::runtime_error("Could not write indexed data record");
}

void Buffer::readAt(std::streamoff offset, ZipCodeRecord& record) {
    input.clear();
    input.seekg(offset);
    if (!input || !next(record))
        throw std::runtime_error("Cannot read record at index offset " + std::to_string(offset));
}
