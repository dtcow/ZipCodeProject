/** @file Main.cpp
 * @brief Gather sequential ZIP records and print alphabetical state extremes.
 */
#include "Buffer.h"
#include "State.h"
#include <iomanip>
#include <iostream>
#include <map>
#include "IndexEntry.h" //for Part 2
#include <vector>

/** @brief Run the ZIP code application
 * @param argc Argument count.
 * @param argv Program name and optional input CSV path.
 * @return Zero on success, one for input errors, two for incorrect usage
 */
int main(int argc, char* argv[]) {
    if (argc > 2) { std::cerr << "Usage: " << argv[0] << " [input.csv]\n"; return 2; }
    
    try {
        Buffer buffer(argc == 2 ? argv[1] : "us_postal_codes.csv"); //
        std::map<std::string, State> states;

        std::vector<IndexEntry> zipIndex;//Part 2:Creating (in RAM) primary key index

        ZipCodeRecord record;

        //Each itertion gathers one record; the complete input is not stored
        while (buffer.next(record)) {
            
            //New addtion for part 2 >>
            //creating one index record for current zip code
            IndexEntry entry;
            entry.zip = record.zip;
            entry.offset = 0; //0 is temporary so we can see that the index will exist in ram
            zipIndex.push_back(entry); //storing the zipcode and index into RAM index
            //<<

            auto it = states.find(record.state);
            if (it == states.end()) 
                states.emplace(record.state, State(record));
            else 
                it->second.update(record);
        }

        std::cout << "Index entries: " << zipIndex.size() << std::endl; //Part 2 work

        //Part 2: Write(as a file) >>
        std::ofstream indexFile("zipcode.idx");// make the index file on disk

        for(const auto & entry : zipIndex){//visit every index entry stored in RAM
            indexFile << entry.zip << "," << entry.offset << "\n";//writing every RAM index entry into the file
        }
        indexFile.close();
        //<<

        //Part 2: Reading index file back into RAM >>
        std::vector<IndexEntry> loadedIndex;
        std::ifstream readIndex("zipcode.idx");

        std::cout << std::left << std::setw(8) << "State"
                  << std::setw(14) << "Easternmost" << std::setw(14) << "Westernmost"
                  << std::setw(14) << "Northernmost" << "Southernmost\n";
        
        for (const auto& entry : states) {
            const auto& state = entry.second;
            std::cout << std::setw(8) << entry.first
                      << std::setw(14) << state.east.zip << std::setw(14) << state.west.zip
                      << std::setw(14) << state.north.zip << state.south.zip << '\n';
        }
    //part 2

    } 
    catch (const std::exception& e){ 
        std::cerr << "Error: " << e.what() << '\n'; return 1;}

    return 0;
}