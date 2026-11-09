/**
 * @file IndexEntry.h
 * @brief This file will the store the information of zipcode/s and an offset which will give us the location of that zipcode within the file
 */
#ifndef INDEXENTRY_H
#define INDEXENTRY_H
#include <string>

struct IndexEntry {

    std::string zip;
    long offset; /*for position of zipcode starting from the file */


};
#endif