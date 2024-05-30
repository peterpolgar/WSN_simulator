#include <vector>
#include <iostream>

#include "placement_function.h"

int main(int argc, char const *argv[])
{
    std::vector<double> tesztvec;
    int ns = 5, nn = 30, w = 160, h = 90, seed = 0;
    Compute_initial_position(&tesztvec, ns, w, h, seed);
    std::cout << "positions_of_sinks,";
    for ( unsigned i = 0; i < tesztvec.size(); ++i ) {
        std::cout << tesztvec[i] << ",";
    }
    std::cout << "\n";
    Compute_initial_position(&tesztvec, nn, w, h, seed);
    std::cout << "positions_of_nodes,";
    for ( unsigned i = 0; i < tesztvec.size(); ++i ) {
        std::cout << tesztvec[i] << ",";
    }
    std::cout << "\n";
    return 0;
}