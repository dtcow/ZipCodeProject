/** @file HeaderBuffer.h
 * @brief Metadata and I/O for version 1 length-indicated ZIP files.
 */
#ifndef ZIP_HEADER_BUFFER_H
#define ZIP_HEADER_BUFFER_H

#include <iosfwd>
#include <string>

/** @brief Per-file header values. Other format properties are fixed in version 1. */
struct DataHeader {
    std::string indexFilename; ///< Name of the companion index in the data directory.
    unsigned long long recordCount = 0; ///< Number of data records following the header.
    std::streamoff headerSize = 0; ///< Byte position of the first record prefix.
};

/** @brief Reads and writes data file headers and fixed-width ASCII size fields. */
class HeaderBuffer {
public:
    static constexpr unsigned int recordSizeWidth = 10; ///< Bytes per data record length.

    /** @brief Read and validate metadata from the start of a data stream.
     * @param input Binary-mode data stream.
     * @return Header metadata. The stream is positioned at the first data record.
     */
    static DataHeader read(std::istream& input);

    /** @brief Write the header at the current output position.
     * @param output Binary-mode stream positioned at the start of the file.
     * @param header Metadata to write.
     */
    static void write(std::ostream& output, const DataHeader& header);

    /** @brief Read exactly width ASCII digits and return their unsigned value. */
    static unsigned long long readSize(std::istream& input, unsigned int width);

    /** @brief Write a size as exactly width zero-padded ASCII digits. */
    static void writeSize(std::ostream& output, unsigned long long size, unsigned int width);
};
#endif
