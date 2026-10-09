#include <string>

struct State
{
    std::string stateID; //State ID of the current state
    std::string northMost; //Northmost place in this state
    std::string westMost; //Westmost place in this state
    std::string southMost; //Southmost place in this state
    std::string eastMost; //Eastmost place in this state

    float northMostLat;
    float southMostLat;
    float westMostLon;
    float eastMostLon;
};

