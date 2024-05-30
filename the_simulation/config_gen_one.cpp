#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "placement_function.h"

int main()
{
    int number_of_nodes[4]{ 30, 60, 90, 120 };
    int number_of_sinks[6]{ 1, 2, 3, 4, 5, 6 };
    int payload_sizes[3]{ 44, 108, 236 };
    double aggs[5]{ 0.1, 0.3, 0.5, 0.7, 0.9 };
    
    int aoi_w = 3200, aoi_h = 1800, sink_seed = 0, node_seed = 2;
    std::vector<double> posvec;
    
    for ( int i = 3; i < 4; ++i ) {
        for ( int j = 0; j < 1; ++j ) {
            for ( int k = 2; k < 3; ++k ) {
                for ( int l = 0; l < 1; ++l ) {
                    std::string file_name{ "one_teszt/g" };
                    std::string num_text = std::to_string(aggs[l]);
                    file_name += num_text.substr(0, num_text.find(".")+2);
                    file_name += "_NS";
                    file_name += std::to_string(number_of_sinks[j]);
                    file_name += "_NN";
                    file_name += std::to_string(number_of_nodes[i]);
                    file_name += "_P";
                    file_name += std::to_string(payload_sizes[k]);
                    file_name += ".conf";
                    // std::cout << file_name << ", ";
                    std::ofstream ki_file{ file_name, std::ios::out };
                    std::string ki_str;
                    Compute_initial_position(&posvec, number_of_sinks[j], aoi_w, aoi_h, sink_seed);
                    ki_str += "positions_of_sinks,";
                    for ( unsigned z = 0; z < posvec.size() - 1; ++z ) {
                        ki_str += std::to_string(posvec[z]);
                        ki_str += ",";
                    }
                    ki_str += std::to_string(posvec[posvec.size() - 1]);
                    Compute_initial_position(&posvec, number_of_nodes[i], aoi_w, aoi_h, node_seed);
                    ki_str += "\n\npositions_of_nodes,";
                    for ( unsigned z = 0; z < posvec.size() - 1; ++z ) {
                        ki_str += std::to_string(posvec[z]);
                        ki_str += ",";
                    }
                    ki_str += std::to_string(posvec[posvec.size() - 1]);
                    ki_str += "\n\nis all node synchronized,true\n";
                    ki_str += "aggregation heterogenity factor,";
                    ki_str += num_text.substr(0, num_text.find(".")+2);
                    ki_str += "\naggregation extreme heterogenity factor,";
                    ki_str += num_text.substr(0, num_text.find(".")+2);
                    ki_str += "\nnumber_of_nodes,";
                    ki_str += std::to_string(number_of_nodes[i]);
                    ki_str += "\nnumber_of_sinks,";
                    ki_str += std::to_string(number_of_sinks[j]);
                    ki_str += "\nframe payload lengths [B],";
                    ki_str += std::to_string(payload_sizes[k]);
                    ki_str += "\n\n# Mobilities\n# Sink mobility\nsink_mobility_alg,Random Walk\nlibrary prefix name,random_walk_mobility_plugin\nseed,555\ntime period t [s],5\ntype of coordinate system,global\ndirection range start [degree],0.000000\ndirection range end [degree],360.000000\nspeed range start [km/h],5\nspeed range end [km/h],50\nspeed distribution,uniform\ncollision avoidance,opposite direction\ncollision avoidance radius [cm],219.000000\n\n# Node mobility\nnode_mobility_alg,Random Walk\nlibrary prefix name,random_walk_mobility_plugin\nseed,777\ntime period t [s],5\ntype of coordinate system,global\ndirection range start [degree],0.000000\ndirection range end [degree],360.000000\nspeed range start [km/h],5\nspeed range end [km/h],50\nspeed distribution,uniform\ncollision avoidance,opposite direction\ncollision avoidance radius [cm],219.000000\n";
                    ki_file.write(ki_str.c_str(), ki_str.size());
                    ki_file.close();
                }
            }
        }
    }
    std::cout << '\n';
}