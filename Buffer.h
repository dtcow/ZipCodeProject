/** @file Buffer.h
 * @brief Sequential CSV input buffer and ZIP code record definition.
 */
#ifndef ZIP_BUFFER_H
#define ZIP_BUFFER_H
#include <fstream>
#include <string>
#include <vector>

/**
@brief Buffer class stores and processes zip code data to determine the cardinal extremes

 */
class Buffer {
private:
    std::unordered_map<std::string, State> cardinalExtremes;

public:

    /* @brief Contructs an empty Buffer object */
    Buffer() {}

    /*@param filename ,Name of the csv file to read from*/
    void PackCardinalExtremes(const std::string& filename); 
    

    /*@brief Prints out the values to a table, with headers that says, StateID, NorthMost, EastMost, SouthMost, WestMost*/
    void print(); 

    //Should print out the values to a table, such that we have head that says, StateID, NorthMost, EastMost, SouthMost, WestMost
    // For Example each line:  MN    | Duluth  | Red Wing | Red Wing | St. Cloud
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
