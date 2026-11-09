/** @file Buffer.h
 * @brief Sequential CSV input buffer and ZIP code record definition.
 */
#ifndef ZIP_BUFFER_H
#define ZIP_BUFFER_H
#include "HeaderBuffer.h"
#include <array>
#include <fstream>
#include <string>
#include <vector>

/** @brief One ZIP code record gathered from a CSV row. */
struct ZipCodeRecord {
    std::string zip;   ///< Five-digit ZIP code, preserving leading zeros.
    std::string place; ///< Place name.
    std::string state; ///< Two-character state or territory ID.
    std::string county; ///< County name from the source row.
    double latitude = 0;  ///< Latitude in degrees.
    double longitude = 0; ///< Longitude in degrees.
};

/** @brief Reads CSV records sequentially, keeping only the current row.
 * Supports quoted fields, escaped quotes, CRLF, and multiline quoted headers.
 * CSV columns are identified by name. Indexed data uses the order in its header.
 */
class Buffer {
    std::ifstream input; ///< Source stream.
    std::size_t recordNumber = 0; ///< Logical CSV record number, including header.

    std::array<std::size_t, 6> columns{}; ///< Source position of each canonical field.
    std::streampos dataEnd{}; ///< End of the indexed data stream for length validation.
    bool lengthIndicated = false; ///< Whether records start with an ASCII byte count.

    /** @brief Read one logical CSV record.
     * @param source Stream containing the CSV data.
     * @param fields Destination for parsed fields.
     * @return False at end of file; true when a record is available.
     */
    bool readFields(std::istream& source, std::vector<std::string>& fields);
public:
    /** @brief Open input and consume one logical header record.
     * @param filename CSV path.
     * @throws std::runtime_error If input or header is invalid.
     */
    explicit Buffer(const std::string& filename);
    
    /** @brief Open a length-indicated file and read its header.
     * @param filename Length-indicated file path.
     * @param header Destination for header metadata.
     */
    Buffer(const std::string& filename, DataHeader& header);

    /** @brief Write one length-indicated ZIP record.
     * @param output Binary-mode data stream positioned after the header or previous record.
     * @param record ZIP record to serialize in the header's canonical field order.
     */
    static void writeRecord(std::ostream& output, const ZipCodeRecord& record);

    /** @brief Seek to an indexed record and unpack it.
     * @param offset Byte position of the record's length prefix.
     * @param record Destination ZIP record.
     */
    void readAt(std::streamoff offset, ZipCodeRecord& record);

    /** @brief Gather the next ZIP record from the sequential stream.
     * @param record Destination ZIP record.
     * @return False at end of file, otherwise true.
     * @throws std::runtime_error For malformed rows or coordinates.
     */
    bool next(ZipCodeRecord& record);
};
#endif