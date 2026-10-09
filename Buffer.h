#include <unordered_map>
#include <string>
#include "State.h"

class Buffer {
private:
    std::unordered_map<std::string, State> cardinalExtremes;

public:
    Buffer() {}

    void PackCardinalExtremes(const std::string& filename); 

    void print(); //Should print out the values to a table, such that we have head that says, StateID, NorthMost, EastMost, SouthMost, WestMost
                                                                    // For Example each line:  MN    | Duluth  | Red Wing | Red Wing | St. Cloud
};
