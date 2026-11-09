/** @file HeaderBuffer.cpp
 * @brief Versioned header serialization and validation.
 */
#include "HeaderBuffer.h"
#include <algorithm>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace {
constexpr char signature[] = "ZIP-LENGTH 1 "; ///< File structure signature and version.
constexpr unsigned int headerSizeWidth = 10; ///< Bytes in the header size field.
constexpr unsigned int countWidth = 20; ///< Bytes in the record count field.

/** @brief Serialize metadata after the signature and total header byte count. */
std::string headerBody(const DataHeader& header) {
    if (header.indexFilename.empty() ||
        header.indexFilename.find_first_of("/\\\r\n") != std::string::npos ||
        header.indexFilename == "." || header.indexFilename == "..")
        throw std::runtime_error("Invalid companion index filename");

    std::ostringstream output;
    output << '\n'
           << "record_size_bytes=" << HeaderBuffer::recordSizeWidth << '\n'
           << "size_format=ASCII\n"
           << "size_includes_prefix=false\n"
           << "index=" << std::quoted(header.indexFilename) << '\n'
           << "record_count=";
    HeaderBuffer::writeSize(output, header.recordCount, countWidth);
    output << "\nfield_count=6\n"
           << "fields=Zip Code:text(5),Place Name:CSV text,State:CSV text(2),"
              "County:CSV text,Lat:ASCII double,Long:ASCII double\n"
           << "primary_key_field=1\n";
    return output.str();
}
}

unsigned long long HeaderBuffer::readSize(std::istream& input, unsigned int width) {
    std::string digits(width, '\0');
    if (!input.read(digits.data(), width) ||
        !std::all_of(digits.begin(), digits.end(), [](char c) { return c >= '0' && c <= '9'; }))
        throw std::runtime_error("Invalid ASCII size field");
    return std::stoull(digits);
}

void HeaderBuffer::writeSize(std::ostream& output, unsigned long long size, unsigned int width) {
    const std::string digits = std::to_string(size);
    if (digits.size() > width) throw std::runtime_error("Value exceeds ASCII size field");
    output << std::string(width - digits.size(), '0') << digits;
}

void HeaderBuffer::write(std::ostream& output, const DataHeader& header) {
    const std::string body = headerBody(header);
    output << signature;
    writeSize(output, sizeof(signature) - 1 + headerSizeWidth + body.size(), headerSizeWidth);
    output << body;
    if (!output) throw std::runtime_error("Could not write data header");
}

DataHeader HeaderBuffer::read(std::istream& input) {
    std::string prefix(sizeof(signature) - 1, '\0');
    if (!input.read(prefix.data(), prefix.size()) || prefix != signature)
        throw std::runtime_error("Unsupported indexed data signature or version");

    const auto size = readSize(input, headerSizeWidth);
    const auto prefixSize = prefix.size() + headerSizeWidth;
    if (size <= prefixSize || size > 65536)
        throw std::runtime_error("Invalid data header size");

    std::string body(size - prefixSize, '\0');
    if (!input.read(body.data(), body.size())) throw std::runtime_error("Truncated data header");
    const auto indexPosition = body.find("\nindex=");
    const auto countPosition = body.find("\nrecord_count=");
    if (indexPosition == std::string::npos || countPosition == std::string::npos)
        throw std::runtime_error("Incomplete data header");

    DataHeader header;
    std::istringstream indexField(body.substr(indexPosition + sizeof("\nindex=") - 1));
    std::istringstream countField(body.substr(countPosition + sizeof("\nrecord_count=") - 1));
    if (!(indexField >> std::quoted(header.indexFilename)))
        throw std::runtime_error("Invalid index filename in data header");
    header.recordCount = readSize(countField, countWidth);
    header.headerSize = static_cast<std::streamoff>(size);
    if (body != headerBody(header)) throw std::runtime_error("Invalid version 1 data header");
    return header;
}
