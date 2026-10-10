#include <unordered_map>
#include <string>
#include "State.h"

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
