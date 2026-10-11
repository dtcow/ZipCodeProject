/** @file Buffer.cpp
 * @brief CSV buffering, parsing, and record validation.
 */
#include "Buffer.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

bool Buffer::readFields(std::vector<std::string>& fields) {
    fields.clear();
    std::string field;

    bool quoted = false, closedQuote = false, started = false;
    char c;

    while (input.get(c)) {
        started = true;
        if (quoted) {
            if (c == '"') {
                if (input.peek() == '"') { input.get(c); field += '"'; }
                else { quoted = false; closedQuote = true; }
            } else { field += c; }
        } else if (c == ',') {
            fields.push_back(field); field.clear(); closedQuote = false;
        } else if (c == '\n' || c == '\r') {
            if (c == '\r' && input.peek() == '\n') input.get(c);
            fields.push_back(field); ++recordNumber; return true;
        } else if (c == '"' && field.empty() && !closedQuote) {
            quoted = true;
        } else {
            if (closedQuote || c == '"')
                throw std::runtime_error("Invalid CSV quoting near record " + std::to_string(recordNumber + 1));
            field += c;
        }
    }

    if (input.bad()) throw std::runtime_error("Failed while reading CSV");
    if (quoted) throw std::runtime_error("Unterminated quoted CSV field");
    if (!started) return false;
    
    fields.push_back(field); ++recordNumber; return true;
}

Buffer::Buffer(const std::string& filename) : input(filename) {
    if (!input) throw std::runtime_error("Cannot open CSV: " + filename);

    //remove an optional UTF-8 BOM before reading the logical header.
    if (input.peek() == 0xEF) {
        char bom[3]; input.read(bom, 3);
        if (!input || static_cast<unsigned char>(bom[1]) != 0xBB ||
            static_cast<unsigned char>(bom[2]) != 0xBF)
            throw std::runtime_error("Invalid UTF-8 BOM");
    }

    std::vector<std::string> header;

    if (!readFields(header) || header.size() != 6)
        throw std::runtime_error("Expected six CSV header columns");

    for (auto& name : header) {
        name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) { return std::isspace(c); }), name.end());
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    const std::vector<std::string> expected{"zipcode", "placename", "state", "county", "lat", "long"};

    if (header != expected) throw std::runtime_error("Unexpected CSV column order or names");
}

bool Buffer::next(ZipCodeRecord& record) {
    std::vector<std::string> fields;

    while (readFields(fields)) {
        if (fields.size() == 1 && fields[0].empty()) continue;

        try {
            if (fields.size() != 6) throw std::runtime_error("Expected six fields");
            record.zip = fields[0];
            if (record.zip.empty() || record.zip.size() > 5 ||
                !std::all_of(record.zip.begin(), record.zip.end(), [](unsigned char c) { return std::isdigit(c); }))
                throw std::runtime_error("Invalid ZIP code");
            record.zip.insert(0, 5 - record.zip.size(), '0');
            record.place = fields[1]; record.state = fields[2];
            if (record.state.size() != 2) throw std::runtime_error("Invalid state ID");
            std::size_t used;
            record.latitude = std::stod(fields[4], &used);
            if (used != fields[4].size() || !std::isfinite(record.latitude) ||
                record.latitude < -90 || record.latitude > 90)
                throw std::runtime_error("Invalid latitude");
            record.longitude = std::stod(fields[5], &used);
            if (used != fields[5].size() || !std::isfinite(record.longitude) ||
                record.longitude < -180 || record.longitude > 180)
                throw std::runtime_error("Invalid longitude");
        } catch (const std::exception& e) {
            throw std::runtime_error("CSV record " + std::to_string(recordNumber) + ": " + e.what());
        }
        return true;
    }
    return false;
}
