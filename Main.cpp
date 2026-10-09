#include "Buffer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

void Buffer::PackCardinalExtremes(const std::string& filename) {
    std::ifstream file(filename);

    std::string line;

    getline(file, line); 
    getline(file, line); 
    getline(file, line); 

    while (getline(file, line)) {
        stringstream lineString(line);

        string zip, stateID, place, county, lat, lon;
        float latitude, longitude;

        getline(lineString, zip, ',');
        getline(lineString, place, ',');
        getline(lineString, stateID, ',');
        getline(lineString, county, ',');
        getline(lineString, lat, ',');
        getline(lineString, lon, ',');

        latitude = stof(lat);
        longitude = stof(lon);

        auto it = cardinalExtremes.find(stateID);
        if (it != cardinalExtremes.end()) { //It found it
            State& currentState = cardinalExtremes[stateID];
            if (currentState.northMostLat< latitude) { //Largest float for Lat
                currentState.northMost = place;
                currentState.northMostLat = latitude;
            }
            if (currentState.southMostLat > longitude) { //Lowest Float for Lat
                currentState.southMost = place;
                currentState.southMostLat = latitude;
            }
            if (currentState.westMostLon > longitude) { //Largest float for Lon
                currentState.westMost = place;
                currentState.westMostLon = longitude;
            }
            if (currentState.eastMostLon < longitude) { //Lowest Float for Lon
                currentState.eastMost = place;
                currentState.westMostLon = longitude;
            }

        } else { //It does not exist
            State state;
            state.stateID = stateID;
            state.northMost = place;
            state.westMost = place;
            state.southMost = place;
            state.eastMost = place;

            state.northMostLat = latitude;
            state.westMostLon = longitude;
            state.southMostLat = latitude;
            state.eastMostLon = longitude;

            cardinalExtremes[stateID] = state;
        }
    }

    file.close();
} 

void Buffer::print() {
    int count = 1;
    for (const auto& pair : cardinalExtremes) {
        cout << count << ": " << pair.first << " | " << pair.second.northMost << " | " << pair.second.eastMost << " | " << pair.second.southMost << " | " << pair.second.westMost << "\n";
        count++;
    }
}

int main() {
    Buffer buffer;

    buffer.PackCardinalExtremes("us_postal_codes.csv");
    buffer.print();
}