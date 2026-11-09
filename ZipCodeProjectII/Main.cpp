/** @file Main.cpp
 * @brief Build ZIP data and index files, look up ZIP codes, or print state extremes.
 */
#include "Buffer.h"
#include "IndexEntry.h"
#include "State.h"
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string_view>

namespace fs = std::filesystem;

/** @brief Check whether a key is exactly five ASCII digits. */
static bool isZip(std::string_view zip) {
    return zip.size() == 5 && std::all_of(zip.begin(), zip.end(), [](char digit) {
        return digit >= '0' && digit <= '9';
    });
}

/** @brief Stream records into the indexed data file and write the sorted primary index.
 * @param csvPath Source CSV, in any supported column order.
 * @param dataPath Destination .zdat file. Its companion index uses the .zdat.idx suffix.
 */
static void buildIndex(const std::string& csvPath, const fs::path& dataPath) {
    if (dataPath.extension() != ".zdat")
        throw std::runtime_error("Use a .zdat filename for indexed data");

    Buffer buffer(csvPath);
    DataHeader header;
    header.indexFilename = dataPath.filename().string() + ".idx";
    std::ofstream data(dataPath, std::ios::binary);
    if (!data) throw std::runtime_error("Cannot create indexed data: " + dataPath.string());
    HeaderBuffer::write(data, header);

    std::vector<IndexEntry> zipIndex;
    ZipCodeRecord record;
    while (buffer.next(record)) {
        const auto offset = data.tellp();
        Buffer::writeRecord(data, record);
        zipIndex.push_back({record.zip, offset});
    }

    std::sort(zipIndex.begin(), zipIndex.end(), [](const IndexEntry& left, const IndexEntry& right) {
        return left.zip < right.zip;
    });
    for (std::size_t i = 1; i < zipIndex.size(); ++i) {
        if (zipIndex[i - 1].zip == zipIndex[i].zip)
            throw std::runtime_error("Duplicate primary key: " + zipIndex[i].zip);
    }

    // The fixed-width record count lets us update the header without moving any records.
    header.recordCount = zipIndex.size();
    data.seekp(0);
    HeaderBuffer::write(data, header);
    data.close();
    if (!data) throw std::runtime_error("Could not finish writing indexed data");

    std::ofstream indexFile(dataPath.parent_path() / header.indexFilename);
    if (!indexFile) throw std::runtime_error("Cannot create companion index");
    for (const auto& entry : zipIndex)
        indexFile << entry.zip << ',' << entry.offset << '\n';
    indexFile.close();
    if (!indexFile) throw std::runtime_error("Could not finish writing companion index");

    std::cout << "Indexed " << header.recordCount << " ZIP codes in " << dataPath.string() << '\n';
}

/** @brief Load sorted ZIP keys and offsets from the companion file.
 * @param dataPath Path to the indexed data file.
 * @param header Data header with the index filename, record count, and minimum offset.
 * @return Sorted primary index for binary search.
 */
static std::vector<IndexEntry> loadIndex(const fs::path& dataPath, const DataHeader& header) {
    const auto indexPath = dataPath.parent_path() / header.indexFilename;
    std::ifstream input(indexPath);
    if (!input) throw std::runtime_error("Cannot open companion index: " + indexPath.string());

    std::vector<IndexEntry> index;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const std::string zip = line.substr(0, 5);
        if (line.size() < 7 || !isZip(zip) || line[5] != ',' ||
            !std::all_of(line.begin() + 6, line.end(), [](char c) { return c >= '0' && c <= '9'; }) ||
            (!index.empty() && index.back().zip >= zip))
            throw std::runtime_error("Invalid or unsorted companion index entry");

        std::size_t used;
        const auto offset = std::stoll(line.substr(6), &used);
        if (offset < header.headerSize || used != line.size() - 6 || index.size() >= header.recordCount)
            throw std::runtime_error("Invalid companion index offset or record count");

        index.push_back({zip, offset});
    }
    if (input.bad() || index.size() != header.recordCount)
        throw std::runtime_error("Companion index does not match the data header");
    return index;
}

/** @brief Quote text and show embedded line breaks without splitting a record. */
static std::string displayText(const std::string& text) {
    std::string quoted = "\"";
    for (char c : text) {
        switch (c) {
            case '\n': quoted += "\\n"; break;
            case '\r': quoted += "\\r"; break;
            case '"': quoted += "\\\""; break;
            case '\\': quoted += "\\\\"; break;
            default: quoted += c;
        }
    }
    return quoted + '"';
}

/** @brief Format a coordinate with the shortest decimal that round-trips to its value. */
static std::string displayCoordinate(double value) {
    char digits[64];
    const auto result = std::to_chars(digits, digits + sizeof(digits), value);
    if (result.ec != std::errc{}) throw std::runtime_error("Could not format coordinate");
    return std::string(digits, result.ptr);
}

/** @brief Load the index, then seek to and print one record per matching -Z option.
 * @param dataPath Path to the length-indicated data file.
 * @param first Beginning of the command-line search options.
 * @param last One past the final option.
 */
static int searchZipCodes(const fs::path& dataPath, char* const* first, char* const* last) {
    for (auto option = first; option != last; ++option) {
        const std::string_view flag(*option);
        if (flag.substr(0, 2) != "-Z" || !isZip(flag.substr(2))) {
            std::cerr << "Invalid ZIP search flag: " << flag << " (use -Z followed by five digits)\n";
            return 2;
        }
    }

    DataHeader header;
    Buffer buffer(dataPath.string(), header);
    const auto index = loadIndex(dataPath, header);
    ZipCodeRecord record;
    for (auto option = first; option != last; ++option) {
        const std::string_view zip(*option + 2);
        auto entry = std::lower_bound(index.begin(), index.end(), zip,
            [](const IndexEntry& candidate, std::string_view key) { return candidate.zip < key; });
        if (entry == index.end() || entry->zip != zip) {
            std::cout << "ZIP Code " << zip << " was not found.\n";
            continue;
        }

        buffer.readAt(entry->offset, record);
        if (record.zip != zip) throw std::runtime_error("Companion index points to the wrong ZIP code");
        std::cout << "ZIP Code: " << record.zip << " | Place Name: " << displayText(record.place)
                  << " | State: " << record.state << " | County: " << displayText(record.county)
                  << " | Latitude: " << displayCoordinate(record.latitude)
                  << " | Longitude: " << displayCoordinate(record.longitude) << '\n';
    }
    return 0;
}

/** @brief Run the original directional-extremes calculation using either input format.
 * @param buffer Open CSV or indexed data buffer.
 */
static void printExtremes(Buffer& buffer) {
    std::map<std::string, State> states;
    ZipCodeRecord record;
    while (buffer.next(record)) {
        auto it = states.find(record.state);
        if (it == states.end())
            states.emplace(record.state, State(record));
        else
            it->second.update(record);
    }

    std::cout << std::left << std::setw(8) << "State"
              << std::setw(14) << "Easternmost" << std::setw(14) << "Westernmost"
              << std::setw(14) << "Northernmost" << "Southernmost\n";
    
    for (const auto& entry : states) {
        const auto& state = entry.second;
        std::cout << std::setw(8) << entry.first
                  << std::setw(14) << state.east.zip << std::setw(14) << state.west.zip
                  << std::setw(14) << state.north.zip << state.south.zip << '\n';
    }
}

/** @brief Run the ZIP code application.
 * @param argc Argument count.
 * @param argv Program name and build, search, or extremes arguments.
 * @return Zero on success, one for data errors, two for incorrect usage.
 */
int main(int argc, char* argv[]) {
    try {
        if (argc == 4 && std::string_view(argv[1]) == "--build") {
            buildIndex(argv[2], argv[3]);
        } else if (argc == 3 && std::string_view(argv[1]) == "--extremes") {
            DataHeader header;
            Buffer buffer(argv[2], header);
            printExtremes(buffer);
        } else if (argc >= 3 && argv[1][0] != '-') {
            return searchZipCodes(argv[1], argv + 2, argv + argc);
        } else if (argc == 1 || (argc == 2 && argv[1][0] != '-')) {
            Buffer buffer(argc == 2 ? argv[1] : "us_postal_codes.csv");
            printExtremes(buffer);
        } else {
            std::cerr << "Usage:\n"
                      << "  " << argv[0] << " [input.csv]\n"
                      << "  " << argv[0] << " --build input.csv output.zdat\n"
                      << "  " << argv[0] << " --extremes input.zdat\n"
                      << "  " << argv[0] << " input.zdat -Z##### [-Z##### ...]\n";
            return 2;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
