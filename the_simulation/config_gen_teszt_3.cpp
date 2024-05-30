#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "placement_function.h"

int main()
{
    std::string mvm_sink[2]{ "None", "Random_Walk" };
    std::string mvm_node[2]{ "None",  "Random_Walk" };
    int number_of_nodes[4]{ 30, 60, 90, 120 };
    int number_of_sinks[6]{ 1, 2, 3, 4, 5, 6 };
    int payload_sizes[3]{ 44, 108, 236 };
    double aggs[5]{ 0.1, 0.3, 0.5, 0.7, 0.9 };
    
    int aoi_w = 3200, aoi_h = 1800, sink_seed = 0, node_seed = 1;
    std::vector<double> posvec;
    
    for ( int i = 0; i < 4; ++i ) {
        for ( int j = 0; j < 6; ++j ) {
            for ( int k = 0; k < 3; ++k ) {
                for ( int l = 0; l < 5; ++l ) {
                    for ( int m = 0; m < 2; ++m ) {
                        std::string file_name{ "twelveth_prod_runs/" };
                        file_name += "SM";
                        file_name += mvm_sink[m];
                        file_name += "_NM";
                        file_name += mvm_node[m];
                        file_name += "_g";
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
                        ki_str += "\n\naoi_meter_width,3200\naoi_meter_height,1800";
                        ki_str += "\nis all node synchronized,true\n";
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
                        ki_str += "\nframe receive energy (Eelec) [μJ/bit],5";
                        ki_str += "\nenergy-free space factor (Efs) [μJ/bit/m^2],0.001";
                        ki_str += "\nvelocity max. [km/h],50";
                        ki_str += "\ninitial batter energy of a sensor node (E0) [μJ],5000000";
                        if ( mvm_sink[m] == "None" ) {
                            ki_str += "\n";
                        }else {
                            ki_str += "\n\n# Mobilities\n# Sink mobility\nsink_mobility_alg,Random Walk\nlibrary prefix name,random_walk_mobility_plugin\nseed,555\ntime period t [s],5\ntype of coordinate system,global\ndirection range start [degree],0.000000\ndirection range end [degree],360.000000\nspeed range start [km/h],5\nspeed range end [km/h],50\nspeed distribution,uniform\ncollision avoidance,opposite direction\ncollision avoidance radius [cm],219.000000\n\n# Node mobility\nnode_mobility_alg,Random Walk\nlibrary prefix name,random_walk_mobility_plugin\nseed,777\ntime period t [s],5\ntype of coordinate system,global\ndirection range start [degree],0.000000\ndirection range end [degree],360.000000\nspeed range start [km/h],5\nspeed range end [km/h],50\nspeed distribution,uniform\ncollision avoidance,opposite direction\ncollision avoidance radius [cm],219.000000\n";
                        }
                        ki_file.write(ki_str.c_str(), ki_str.size());
                        ki_file.close();
                    }
                }
            }
        }
    }
}