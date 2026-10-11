/** @file Main.cpp
 * @brief Gather sequential ZIP records and print alphabetical state extremes.
 */
#include "Buffer.h"
#include "State.h"
#include <iomanip>
#include <iostream>
#include <map>

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
        ZipCodeRecord record;

        //Each itertion gathers one record; the complete input is not stored
        while (buffer.next(record)) {
            auto it = states.find(record.state);
            if (it == states.end()) states.emplace(record.state, State(record));
            else it->second.update(record);
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
    } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << '\n'; return 1; }
    
    return 0;
}
