/** @file IndexEntry.h
 * @brief Key and byte position for one ZIP code in the data file.
 */
#ifndef INDEXENTRY_H
#define INDEXENTRY_H
#include <ios>
#include <string>

/** @brief One primary key index entry. */
struct IndexEntry {
    std::string zip; ///< Five-digit ZIP code.
    std::streamoff offset; ///< Byte position of the record's length prefix.
};
#endif
