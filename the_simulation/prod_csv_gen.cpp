#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "eparams.h"

int main(int argc, char const *argv[])
{
    if ( argc < 4 ) {
        std::cerr << "Give input file, time interval, and what parameters you want to write out. Exit.\n";
        exit(1);
    }
    
    bool is_number_of_measurements = false;
    bool is_number_of_extr_measurements = false;
    bool is_number_of_received_frames_on_sinks = false;
    bool is_average_hopcount_of_received_frames = false;
    bool is_average_sending_time_of_received_frames = false;
    bool is_rel_deli_error = false;
    bool is_average_energy_of_nodes = false;
    bool is_average_comm_energy = false;
    bool is_sink_positions = false;
    bool is_node_positions = false;
    bool is_alive_nodes_percent = false;
    
    for ( int i = 3; i < argc; ++i ) {
        if ( strcmp(argv[i], "number_of_measurements") == 0 ) {
            is_number_of_measurements = true;
        }else if ( strcmp(argv[i], "number_of_extreme_measurements") == 0 ) {
            is_number_of_extr_measurements = true;
        }else if ( strcmp(argv[i], "number_of_received_frames_on_sinks") == 0 ) {
            is_number_of_received_frames_on_sinks = true;
        }else if ( strcmp(argv[i], "average_hopcount_of_received_frames") == 0 ) {
            is_average_hopcount_of_received_frames = true;
        }else if ( strcmp(argv[i], "average_sending_time_of_received_frames") == 0 ) {
            is_average_sending_time_of_received_frames = true;
        }else if ( strcmp(argv[i], "relative_delivery_error") == 0 ) {
            is_rel_deli_error = true;
        }else if ( strcmp(argv[i], "average_energy_of_nodes") == 0 ) {
            is_average_energy_of_nodes = true;
        }else if ( strcmp(argv[i], "average_comm_energy") == 0 ) {
            is_average_comm_energy = true;
        }else if ( strcmp(argv[i], "sink_positions") == 0 ) {
            is_sink_positions = true;
        }else if ( strcmp(argv[i], "node_positions") == 0 ) {
            is_node_positions = true;
        }else if ( strcmp(argv[i], "percentage_of_alive_nodes") == 0 ) {
            is_alive_nodes_percent = true;
        }else {
            std::cerr << argv[i] << " is not a valid parameter name. Exit.\n";
            exit(2);
        }
    }
    
    eParams tmp_enum;
    double avg_count_of_all_measurements = 0;
    double avg_count_of_all_extr_measurements = 0;
    double act_number_of_received_frames_on_sinks = 0;
    double avg_hopcount_of_rec_frames = 0;
    double avg_sending_time_of_rec_frames = 0;
    double relative_delivery_error = 0;
    double avg_energy_of_nodes = 0;
    double avg_comm_energy = 0;
    std::vector<double> sink_poziciok;
    std::vector<double> node_poziciok;
    double perc_of_alive_nodes = 0;
    
    unsigned long long idobelyeg = 0, prev_idobelyeg = 0, next_time_stamp = idobelyeg;
    unsigned period_time_msec = 1;
    unsigned long out_len = 1, tmp_len = 0;
    int time_interval_msec = std::atoi(argv[2]);
    
    std::ifstream is{argv[1], std::ios::in | std::ios::binary};
    if ( ! is ) {
        std::cerr << "Cannot open data file " << argv[1] << ". Exit.\n";
        exit(3);
    }
    
    char csv_name[155];
    strcpy(csv_name, argv[1]);
    for ( int i = 0; i < 154; ++i ) {
        if ( csv_name[i] == '.' && csv_name[i + 1] == 'o' ) {
            csv_name[i] = '\0';
            break;
        }
    }
    strcat(csv_name, ".csv");
    std::ofstream ki_file{ csv_name, std::ios::out };
    std::string ki_str{ "Time-stamp [msec]" }, tmp_str;
    if ( is_number_of_measurements ) {
        ki_str += ";average number of samplings per node";
    }
    if ( is_average_hopcount_of_received_frames ) {
        ki_str += ";average hopcount of received frames";
    }
    if ( is_average_sending_time_of_received_frames ) {
        ki_str += ";average sending time of received frames [msec]";
    }
    if ( is_number_of_received_frames_on_sinks ) {
        ki_str += ";average number of received sourced frames per node";
    }
    if ( is_rel_deli_error ) {
        ki_str += ";relative delivery error [%]";
    }
    if ( is_average_energy_of_nodes ) {
        ki_str += ";average energy of nodes [%]";
    }
    if ( is_average_comm_energy ) {
        ki_str += ";average communication energy of nodes [%]";
    }
    if ( is_sink_positions ) {
        ki_str += ";positions of sinks [m]";
    }
    if ( is_node_positions ) {
        ki_str += ";positions of nodes [m]";
    }
    if ( is_number_of_extr_measurements ) {
        ki_str += ";average number of extreme samplings per node";
    }
    if ( is_alive_nodes_percent ) {
        ki_str += ";rate of alive nodes [%]";
    }
    ki_str += "\n";
    ki_file.write(ki_str.c_str(), ki_str.size());
    // int count = 1;
    while ( out_len ) {
        // std::cout << count << ". sor:\n";
        is.read((char*)&idobelyeg, sizeof(idobelyeg));
        while ( idobelyeg > next_time_stamp ) {
            if ( next_time_stamp % time_interval_msec == 0 ) {
                std::string uu_str = std::to_string(next_time_stamp) + tmp_str;
                ki_file.write(uu_str.c_str(), uu_str.size());
            }
            next_time_stamp += period_time_msec;
        }
        next_time_stamp += period_time_msec;
        ki_str = std::to_string(idobelyeg);
        tmp_str = "";
        is.read((char*)&out_len, sizeof(out_len));
        // if ( idobelyeg < 20 ) {
            // std::cout << idobelyeg << ' ' << out_len << "\n" << std::flush;
        // }
        unsigned long ki_len = out_len;
        while ( ki_len ) {
            is.read((char*)&tmp_enum, sizeof(eParams::number_of_measurements));
            ki_len -= sizeof(eParams::number_of_measurements);
            if ( tmp_enum == eParams::number_of_measurements ) {
                is.read((char*)&avg_count_of_all_measurements, sizeof(avg_count_of_all_measurements));
                ki_len -= sizeof(avg_count_of_all_measurements);
                // std::cout << "meas\n";
            }else if ( tmp_enum == eParams::number_of_extreme_measurements ) {
                is.read((char*)&avg_count_of_all_extr_measurements, sizeof(avg_count_of_all_extr_measurements));
                ki_len -= sizeof(avg_count_of_all_extr_measurements);
                // std::cout << "meas\n";
            }else if ( tmp_enum == eParams::number_of_received_frames_on_sinks ) {
                is.read((char*)&act_number_of_received_frames_on_sinks, sizeof(act_number_of_received_frames_on_sinks));
                ki_len -= sizeof(act_number_of_received_frames_on_sinks);
                // std::cout << "recframe\n";
            }else if ( tmp_enum == eParams::average_hopcount_of_received_frames ) {
                is.read((char*)&avg_hopcount_of_rec_frames, sizeof(avg_hopcount_of_rec_frames));
                ki_len -= sizeof(avg_hopcount_of_rec_frames);
                // std::cout << "hopc\n";
            }else if ( tmp_enum == eParams::average_sending_time_of_received_frames ) {
                is.read((char*)&avg_sending_time_of_rec_frames, sizeof(avg_sending_time_of_rec_frames));
                ki_len -= sizeof(avg_sending_time_of_rec_frames);
                // std::cout << "sendingtime\n";
            }else if ( tmp_enum == eParams::relative_delivery_error ) {
                is.read((char*)&relative_delivery_error, sizeof(relative_delivery_error));
                ki_len -= sizeof(relative_delivery_error);
                // std::cout << "sendingtime\n";
            }else if ( tmp_enum == eParams::average_energy_of_nodes ) {
                is.read((char*)&avg_energy_of_nodes, sizeof(avg_energy_of_nodes));
                ki_len -= sizeof(avg_energy_of_nodes);
                // std::cout << "sendingtime\n";
            }else if ( tmp_enum == eParams::average_comm_energy ) {
                is.read((char*)&avg_comm_energy, sizeof(avg_comm_energy));
                ki_len -= sizeof(avg_comm_energy);
                // std::cout << "sendingtime\n";
            }else if ( tmp_enum == eParams::sink_positions ) {
                is.read((char*)&tmp_len, sizeof(tmp_len));
                sink_poziciok.resize(tmp_len);
                is.read((char*)sink_poziciok.data(), tmp_len * sizeof(double));
                ki_len -= sizeof(tmp_len) + tmp_len * sizeof(double);
            }else if ( tmp_enum == eParams::node_positions ) {
                is.read((char*)&tmp_len, sizeof(tmp_len));
                node_poziciok.resize(tmp_len);
                is.read((char*)node_poziciok.data(), tmp_len * sizeof(double));
                ki_len -= sizeof(tmp_len) + tmp_len * sizeof(double);
            }else if ( tmp_enum == eParams::percentage_of_alive_nodes ) {
                is.read((char*)&perc_of_alive_nodes, sizeof(perc_of_alive_nodes));
                ki_len -= sizeof(perc_of_alive_nodes);
                // std::cout << "sendingtime\n";
            }
        }
        if ( (out_len && idobelyeg % time_interval_msec == 0) || (out_len == 0 && idobelyeg != prev_idobelyeg) ) {
            if ( is_number_of_measurements ) {
                tmp_str += ";" + std::to_string(avg_count_of_all_measurements);
            }
            if ( is_average_hopcount_of_received_frames ) {
                tmp_str += ";" + std::to_string(avg_hopcount_of_rec_frames);
            }
            if ( is_average_sending_time_of_received_frames ) {
                tmp_str += ";" + std::to_string(avg_sending_time_of_rec_frames);
            }
            if ( is_number_of_received_frames_on_sinks ) {
                tmp_str += ";" + std::to_string(act_number_of_received_frames_on_sinks);
            }
            if ( is_rel_deli_error ) {
                tmp_str += ";" + std::to_string(relative_delivery_error);
            }
            if ( is_average_energy_of_nodes ) {
                tmp_str += ";" + std::to_string(avg_energy_of_nodes);
            }
            if ( is_average_comm_energy ) {
                tmp_str += ";" + std::to_string(avg_comm_energy);
            }
            if ( is_sink_positions ) {
                tmp_str += ";" + std::to_string(sink_poziciok[0]);
                for ( unsigned j = 1; j < sink_poziciok.size(); ++j ) {
                    tmp_str += "," + std::to_string(sink_poziciok[j]);
                }
            }
            if ( is_node_positions ) {
                tmp_str += ";" + std::to_string(node_poziciok[0]);
                for ( unsigned j = 1; j < node_poziciok.size(); ++j ) {
                    tmp_str += "," + std::to_string(node_poziciok[j]);
                }
            }
            if ( is_number_of_extr_measurements ) {
                tmp_str += ";" + std::to_string(avg_count_of_all_extr_measurements);
            }
            if ( is_alive_nodes_percent ) {
                tmp_str += ";" + std::to_string(perc_of_alive_nodes);
            }
            tmp_str += "\n";
            ki_str = ki_str + tmp_str;
            ki_file.write(ki_str.c_str(), ki_str.size());
            prev_idobelyeg = idobelyeg;
        }
        // ++count;
        // std::cout << '\n';
    }
    ki_file.close();
    is.close();
}