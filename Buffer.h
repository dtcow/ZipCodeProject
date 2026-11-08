/** @file Buffer.h
 * @brief Sequential CSV input buffer and ZIP code record definition.
 */
#ifndef ZIP_BUFFER_H
#define ZIP_BUFFER_H
#include <fstream>
#include <string>
#include <vector>

/** @brief One ZIP code record gathered from a CSV row. */
struct ZipCodeRecord {
    std::string zip;   ///< Five-digit ZIP code, preserving leading zeros.
    std::string place; ///< Place name.
    std::string state; ///< Two-character state or territory ID.
    double latitude = 0;  ///< Latitude in degrees.
    double longitude = 0; ///< Longitude exactly as supplied in the CSV.
};

/** @brief Reads CSV records sequentially, keeping only the current row.
 * Supports quoted fields, escaped quotes, CRLF, and multiline quoted headers.
 * The required column order is ZIP, place, state, county, latitude, longitude.
 */
class Buffer {
    std::ifstream input; ///< Source stream.
    std::size_t recordNumber = 0; ///< Logical CSV record number, including header.

    /** @brief Read one logical CSV record.
     * @param fields Destination for parsed fields.
     * @return False at end of file; true when a record is available.
     */
    bool readFields(std::vector<std::string>& fields);
public:
    /** @brief Open input and consume one logical header record.
     * @param filename CSV path.
     * @throws std::runtime_error If input or header is invalid.
     */
    explicit Buffer(const std::string& filename);
    
    /** @brief Gather the next ZIP record from the sequential stream.
     * @param record Destination ZIP record.
     * @return False at end of file, otherwise true.
     * @throws std::runtime_error For malformed rows or coordinates.
     */
    bool next(ZipCodeRecord& record);
};
#endif