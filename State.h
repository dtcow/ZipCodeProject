/** @file State.h
 * @brief Four directional ZIP code candidates per state.
 */
#ifndef ZIP_STATE_H
#define ZIP_STATE_H
#include "Buffer.h"

/** @brief Stores the current extreme records for a state.
 * Longitude follows the assignment literally: east = minimum, west = maximum.
 * Coordinate ties select the smallest ZIP so input order cannot change output.
 */
struct State {
    ZipCodeRecord east;  //< Minimum longitude candidate.
    ZipCodeRecord west;  //< Maximum longitude candidate.
    ZipCodeRecord north; //< Maximum latitude candidate.
    ZipCodeRecord south; //< Minimum latitude candidate.

    /** @brief Initialize all candidates using the state's first record.
     * @param record Initial ZIP record.
     */
    explicit State(const ZipCodeRecord& record) : east(record), west(record), north(record), south(record) {}
    
    /** @brief Compare a record with all four current extremes.
     * @param record New ZIP record for this state.
     */
    void update(const ZipCodeRecord& record) {
        // ZIP tie-breaking is deterministic across different CSV row orders.
        if (record.longitude < east.longitude || (record.longitude == east.longitude && record.zip < east.zip)) east = record;
        if (record.longitude > west.longitude || (record.longitude == west.longitude && record.zip < west.zip)) west = record;
        if (record.latitude > north.latitude || (record.latitude == north.latitude && record.zip < north.zip)) north = record;
        if (record.latitude < south.latitude || (record.latitude == south.latitude && record.zip < south.zip)) south = record;
    }
};
#endif
