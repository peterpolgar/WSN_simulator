#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "eparams.h"
#include "plugins/mobility/mobility_interface.h"
#include "plugins/routing/routing_interface.h"

int main(int argc, char const *argv[])
{
    // initialize settable (by configuration file) parameters with default values
    // AoI ------------------------------------------------------------------------------
    double con_aoi_width_meter = 160;
    double con_aoi_height_meter = 90;
    
    // Sinks ------------------------------------------------------------------------------
    unsigned con_number_of_sinks = 5;
    double con_sink_diameter_meter = 0.2;
    std::vector<double> position_of_sinks_meter;
    
    // Nodes ------------------------------------------------------------------------------
    unsigned con_number_of_nodes = 30;
    double con_nodes_diameter_meter = 0.1;
    std::vector<double> position_of_nodes_meter;
    
    // Communication ------------------------------------------------------------------------------
    double con_b_path_loss_exp = 3;
    double con_frame_header_length_B = 20;
    double con_frame_payload_sizes_B = 44;
    // std::vector<double> con_frame_payload_sizes_B;
    // con_frame_payload_sizes_B.push_back(44);
    // con_frame_payload_sizes_B.push_back(108);
    // con_frame_payload_sizes_B.push_back(236);
    double con_frame_receive_energy_Eelec_mikro_J_per_bit = 0.05;
    double con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter = 0.00001;
    double con_receiving_node_sensitivity_Emin_mikro_J = 2500000 * 0.00001;
    double con_D0_length_meter = 87.5;
    
    // General --------------------------------------------------------------------------------------------------------
    double con_velocity_max_km_h = 50.;
    double con_init_battery_energy_basic_E0_mikro_J = 2500000;
    double con_metric_alpha = 0.95;
    double con_enhanced_nodes_rate = 0.5;
    double con_enhanced_nodes_energy_multiplier = 1.25;
    double con_wakeful_prob = 0.8;
    double con_on_mode_energy_mikro_J_per_s = 0.017 * 1000000 * 3.25;  // based on waspmote: https://www.libelium.com/iot-products/waspmote and https://www.electronicdesign.com/technologies/test-measurement/article/21806680/check-and-control-iot-power-consumption
    double con_sleep_mode_energy_mikro_J_per_s = 0.00003 * 1000000 * 0.5;  // based on waspmote: https://www.libelium.com/iot-products/waspmote and https://www.electronicdesign.com/technologies/test-measurement/article/21806680/check-and-control-iot-power-consumption
    double con_hibernate_mode_energy_mikro_J_per_s = 0.000007 * 1000000 * 0.5;  // based on waspmote: https://www.libelium.com/iot-products/waspmote and https://www.electronicdesign.com/technologies/test-measurement/article/21806680/check-and-control-iot-power-consumption
    double con_on_to_sleep_time_msec = 10;
    double con_sleep_to_hiber_time_msec = 5;
    double con_hiber_to_sleep_time_msec = 5;
    double con_sleep_to_on_time_msec = 10;
    double on_time_default_msec = 5000;
    double sleep_time_default_msec = 45;
    double hiber_time_default_msec = 45;
    unsigned how_many_sleep_hibernate_cycles_in_one_T = 549; // including sleep_to_hiber and hiber_to_sleep transition times.
    double aggregation_heterogenity_factor = 0.5;
    double aggregation_extrem_heterogenity_factor = 0.9;
    bool is_nodes_sync = false;
    bool is_simultaneous_meas_and_comm = false;
    
    // Measurement ------------------------------------------------------------------------------
    double con_measurement_energy_mikro_J = 250; // ?
    double con_measurement_prob = 0.5;
    double con_measurement_time_msec = 400;
    double prob_of_extreme_measurement_event = 0.5;
    double extr_meas_invalid_time_delta_msec = 500.;
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // read the configuration file into a string
    if ( argc < 2 ) {
        std::cerr << "No config file is given. Give a config file name as command line argument. Exit.\n";
        exit(5);
    }
    int c_id = 1;
    const char* env_id_p = std::getenv("SLURM_ARRAY_TASK_ID");
    if ( env_id_p == nullptr ) {
        std::cout << "Environment variable SLURM_ARRAY_TASK_ID is not set. Using the first given config file name argument.\n";
    }else {
        c_id = atoi(env_id_p);
    }
    char in_file_name[155], common_file_name_prefix[155];
    strcpy(common_file_name_prefix, argv[c_id]);
    for ( int i = 0; i < 154; ++i ) {
        if ( common_file_name_prefix[i] == '.' && common_file_name_prefix[i + 1] == 'c' ) {
            common_file_name_prefix[i] = '\0';
            break;
        }
    }
    strcpy(in_file_name, common_file_name_prefix);
    strcat(in_file_name, ".conf");
    char *conf_str = new char[20000];
    std::iostream::pos_type conf_size;
    if (std::ifstream is{in_file_name, std::ios::in | std::ios::ate}) {
        conf_size = is.tellg();
        conf_str[conf_size] = '\0';
        is.seekg(0);
        if (! is.read(&conf_str[0], conf_size)) {
            std::cerr << "Error while reading the given configuration file. Exit.\n";
            exit(1);
        }
    }else {
        std::cerr << "Error while opening the given configuration file. Exit.\n";
        exit(2);
    }
    
    // std::cout << conf_str;
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // Set parameters from config string ------------------------------------------------------------------------------
    
    bool is_there_sink_mobility = false;
    bool is_there_node_mobility = false;
    std::vector<std::string> sink_mobi_para_name_str_vec;
    std::vector<std::string> sink_mobi_para_value_str_vec;
    std::vector<std::string> node_mobi_para_name_str_vec;
    std::vector<std::string> node_mobi_para_value_str_vec;
    char sink_mobility_lib_prefix[80];
    char node_mobility_lib_prefix[80];
    char routing_lib_prefix[80];
    strcpy(sink_mobility_lib_prefix, "./plugins/mobility/");
    strcpy(node_mobility_lib_prefix, "./plugins/mobility/");
    strcpy(routing_lib_prefix, "./plugins/routing/");
    bool is_next_sink = true;
    
    {
    char buffer[60], para_name[60];
    bool was_comma = false;
    int start_pos = 0;
    std::vector<std::string> vv;
    for ( int i = 0; i < conf_size; ++i ) {
        if ( conf_str[i] == '\n' ) {
            if ( was_comma ) {
                strncpy(buffer, conf_str + start_pos, i - start_pos);
                buffer[i - start_pos] = '\0';
                vv.push_back(std::string{ buffer });
                // std::cout << vv[0] << '\n';
                if ( strcmp(para_name, "aoi_meter_width") == 0 ) {
                    con_aoi_width_meter = stod(vv[0]);
                }else if ( strcmp(para_name, "aoi_meter_height") == 0 ) {
                    con_aoi_height_meter = stod(vv[0]);
                }else if ( strcmp(para_name, "number_of_sinks") == 0 ) {
                    con_number_of_sinks = stoi(vv[0]);
                }else if ( strcmp(para_name, "sink_meter_diam") == 0 ) {
                    con_sink_diameter_meter = stod(vv[0]);
                }else if ( strcmp(para_name, "positions_of_sinks") == 0 ) {
                    for ( unsigned long j = 0; j < vv.size(); ++j ) {
                        position_of_sinks_meter.push_back(stod(vv[j]));
                    }
                }else if ( strcmp(para_name, "number_of_nodes") == 0 ) {
                    con_number_of_nodes = stoi(vv[0]);
                }else if ( strcmp(para_name, "node_meter_diam") == 0 ) {
                    con_nodes_diameter_meter = stod(vv[0]);
                }else if ( strcmp(para_name, "positions_of_nodes") == 0 ) {
                    for ( unsigned long j = 0; j < vv.size(); ++j ) {
                        position_of_nodes_meter.push_back(stod(vv[j]));
                    }
                }else if ( strcmp(para_name, "initial batter energy of a sensor node (E0) [μJ]") == 0 ) {
                    con_init_battery_energy_basic_E0_mikro_J = stod(vv[0]);
                }else if ( strcmp(para_name, "metric alpha") == 0 ) {
                    con_metric_alpha = stod(vv[0]);
                }else if ( strcmp(para_name, "enhanced nodes rate") == 0 ) {
                    con_enhanced_nodes_rate = stod(vv[0]);
                }else if ( strcmp(para_name, "enhanced nodes energy multiplier") == 0 ) {
                    con_enhanced_nodes_energy_multiplier = stod(vv[0]);
                }else if ( strcmp(para_name, "wakeful probability") == 0 ) {
                    con_wakeful_prob = stod(vv[0]);
                }else if ( strcmp(para_name, "b path loss exponent") == 0 ) {
                    con_b_path_loss_exp = stod(vv[0]);
                }else if ( strcmp(para_name, "frame header length [B]") == 0 ) {
                    con_frame_header_length_B = stod(vv[0]);
                }else if ( strcmp(para_name, "frame payload lengths [B]") == 0 ) {
                    // con_frame_payload_sizes_B.clear();
                    // for ( unsigned j = 0; j < vv.size(); ++j ) {
                    //     con_frame_payload_sizes_B.push_back(stod(vv[j]));
                    // }
                    con_frame_payload_sizes_B = stod(vv[0]);
                }else if ( strcmp(para_name, "frame receive energy (Eelec) [μJ/bit]") == 0 ) {
                    con_frame_receive_energy_Eelec_mikro_J_per_bit = stod(vv[0]);
                }else if ( strcmp(para_name, "energy-free space factor (Efs) [μJ/bit/m^2]") == 0 ) {
                    con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter = stod(vv[0]);
                }else if ( strcmp(para_name, "receiving node sensitivity (Emin) [μJ]") == 0 ) {
                    con_receiving_node_sensitivity_Emin_mikro_J = stod(vv[0]);
                }else if ( strcmp(para_name, "D0 [m]") == 0 ) {
                    con_D0_length_meter = stod(vv[0]);
                }else if ( strcmp(para_name, "measurement energy [μJ]") == 0 ) {
                    con_measurement_energy_mikro_J = stod(vv[0]);
                }else if ( strcmp(para_name, "measurement probability") == 0 ) {
                    con_measurement_prob = stod(vv[0]);
                }else if ( strcmp(para_name, "On mode energy [μJ]") == 0 ) {
                    con_on_mode_energy_mikro_J_per_s = stod(vv[0]);
                }else if ( strcmp(para_name, "sleep mode energy [μJ]") == 0 ) {
                    con_sleep_mode_energy_mikro_J_per_s = stod(vv[0]);
                }else if ( strcmp(para_name, "hibernate mode energy [μJ]") == 0 ) {
                    con_hibernate_mode_energy_mikro_J_per_s = stod(vv[0]);
                }else if ( strcmp(para_name, "on to sleep time [msec]") == 0 ) {
                    con_on_to_sleep_time_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "sleep to hibernate time [msec]") == 0 ) {
                    con_sleep_to_hiber_time_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "hibernate to sleep time [msec]") == 0 ) {
                    con_hiber_to_sleep_time_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "sleep to On time [msec]") == 0 ) {
                    con_sleep_to_on_time_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "on time default [msec]") == 0 ) {
                    on_time_default_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "sleep time default [msec]") == 0 ) {
                    sleep_time_default_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "hibernate time default [msec]") == 0 ) {
                    hiber_time_default_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "how many sleep-hibernate cycles in one T") == 0 ) {
                    how_many_sleep_hibernate_cycles_in_one_T = stoi(vv[0]);
                }else if ( strcmp(para_name, "aggregation heterogenity factor") == 0 ) {
                    aggregation_heterogenity_factor = stod(vv[0]);
                }else if ( strcmp(para_name, "aggregation extreme heterogenity factor") == 0 ) {
                    aggregation_extrem_heterogenity_factor = stod(vv[0]);
                }else if ( strcmp(para_name, "velocity max. [km/h]") == 0 ) {
                    con_velocity_max_km_h = stod(vv[0]);
                }else if ( strcmp(para_name, "is all node synchronized") == 0 ) {
                    if ( vv[0][0] == 't' ) {
                        is_nodes_sync = true;
                    }
                }else if ( strcmp(para_name, "is simultaneous measurement and communication") == 0 ) {
                    if ( vv[0][0] == 't' ) {
                        is_simultaneous_meas_and_comm = true;
                    }
                }else if ( strcmp(para_name, "measurement time [msec]") == 0 ) {
                    con_measurement_time_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "probability of extreme measurement event") == 0 ) {
                    prob_of_extreme_measurement_event = stod(vv[0]);
                }else if ( strcmp(para_name, "extreme measurement invalid time-delta [msec]") == 0 ) {
                    extr_meas_invalid_time_delta_msec = stod(vv[0]);
                }else if ( strcmp(para_name, "sink_mobility_alg") == 0 ) {
                    is_next_sink = true;
                    is_there_sink_mobility = true;
                }else if ( strcmp(para_name, "node_mobility_alg") == 0 ) {
                    is_next_sink = false;
                    is_there_node_mobility = true;
                }else if ( strcmp(para_name, "library prefix name") == 0 ) {
                    if ( is_next_sink ) {
                        strcpy(sink_mobility_lib_prefix + 19, vv[0].c_str());
                        if ( sink_mobility_lib_prefix[18 + vv[0].size()] == '\r' ) {
                            sink_mobility_lib_prefix[18 + vv[0].size()] = '\0';
                        }
                    }else {
                        strcpy(node_mobility_lib_prefix + 19, vv[0].c_str());
                        if ( node_mobility_lib_prefix[18 + vv[0].size()] == '\r' ) {
                            node_mobility_lib_prefix[18 + vv[0].size()] = '\0';
                        }
                    }
                }else {
                    if ( is_next_sink ) {
                        sink_mobi_para_name_str_vec.push_back(para_name);
                        sink_mobi_para_value_str_vec.push_back(vv[0]);
                    }else {
                        node_mobi_para_name_str_vec.push_back(para_name);
                        node_mobi_para_value_str_vec.push_back(vv[0]);
                    }
                }
                vv.clear();
                was_comma = false;
            }
            start_pos = i + 1;
        }else if ( conf_str[i] == ',' ) {
            if ( was_comma ) {
                strncpy(buffer, conf_str + start_pos, i - start_pos);
                buffer[i - start_pos] = '\0';
                vv.push_back(std::string{ buffer });
            }else {
                strncpy(para_name, conf_str + start_pos, i - start_pos);
                para_name[i - start_pos] = '\0';
            }
            was_comma = true;
            start_pos = i + 1;
        }
    }
    }
    
    // Write out the settable parameters ---------------------------------------------------------------------------------------------------------
    
    std::cout << "All settable (by configuration file) parameters:\n";
    std::cout << "# AoI\n";
    std::cout << "aoi_meter_width: " << con_aoi_width_meter << '\n';
    std::cout << "aoi_meter_height: " << con_aoi_height_meter << "\n\n";
    std::cout << "# Sink(s)\n";
    std::cout << "number_of_sinks: " << con_number_of_sinks << '\n';
    std::cout << "sink_meter_diam: " << con_sink_diameter_meter << "\npositions_of_sinks: ";
    for ( unsigned long i = 0; i < position_of_sinks_meter.size(); ++i ) {
        std::cout << position_of_sinks_meter[i] << ", ";
    }
    std::cout << "\n\n";
    std::cout << "# Nodes\n";
    std::cout << "number_of_nodes: " << con_number_of_nodes << '\n';
    std::cout << "node_meter_diam: " << con_nodes_diameter_meter << "\npositions_of_nodes: ";
    for ( unsigned long i = 0; i < position_of_nodes_meter.size(); ++i ) {
        std::cout << position_of_nodes_meter[i] << ", ";
    }
    std::cout << "\n\n";
    std::cout << "# Communication\n";
    std::cout << "b path loss exponent: " << con_b_path_loss_exp << '\n';
    std::cout << "frame header length [B]: " << con_frame_header_length_B << '\n';
    std::cout << "frame payload lengths [B]: " << con_frame_payload_sizes_B << '\n';
    // std::cout << "frame payload lengths [B]: ";
    // for ( unsigned i = 0; i < con_frame_payload_sizes_B.size(); ++i ) {
    //     std::cout << con_frame_payload_sizes_B[i] << ", ";
    // }
    // std::cout << "\n";
    std::cout << "frame receive energy (Eelec) [μJ/bit]: " << con_frame_receive_energy_Eelec_mikro_J_per_bit << '\n';
    std::cout << "energy-free space factor (Efs) [μJ/bit/m^2]: " << con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter << '\n';
    std::cout << "receiving node sensitivity (Emin) [μJ]: " << con_receiving_node_sensitivity_Emin_mikro_J << '\n';
    std::cout << "D0 [m]: " << con_D0_length_meter << "\n\n";
    std::cout << "# General parameters\n";
    std::cout << "velocity max. [km/h]: " << con_velocity_max_km_h << '\n';
    std::cout << "initial batter energy of a sensor node (E0) [μJ]: " << con_init_battery_energy_basic_E0_mikro_J << '\n';
    std::cout << "metric alpha: " << con_metric_alpha << '\n';
    std::cout << "enhanced nodes rate: " << con_enhanced_nodes_rate << '\n';
    std::cout << "enhanced nodes energy multiplier: " << con_enhanced_nodes_energy_multiplier << '\n';
    std::cout << "wakeful probability: " << con_wakeful_prob << '\n';
    std::cout << "On mode energy [μJ]: " << con_on_mode_energy_mikro_J_per_s << '\n';
    std::cout << "sleep mode energy [μJ]: " << con_sleep_mode_energy_mikro_J_per_s << '\n';
    std::cout << "hibernate mode energy [μJ]: " << con_hibernate_mode_energy_mikro_J_per_s << '\n';
    std::cout << "on to sleep time [msec]: " << con_on_to_sleep_time_msec << '\n';
    std::cout << "sleep to hibernate time [msec]: " << con_sleep_to_hiber_time_msec << '\n';
    std::cout << "hibernate to sleep time [msec]: " << con_hiber_to_sleep_time_msec << '\n';
    std::cout << "sleep to On time [msec]: " << con_sleep_to_on_time_msec << '\n';
    std::cout << "on time default [msec]: " << on_time_default_msec << '\n';
    std::cout << "sleep time default [msec]: " << sleep_time_default_msec << '\n';
    std::cout << "hibernate time default [msec]: " << hiber_time_default_msec << '\n';
    std::cout << "how many sleep-hibernate cycles in one T: " << how_many_sleep_hibernate_cycles_in_one_T << '\n';
    std::cout << "aggregation heterogenity factor: " << aggregation_heterogenity_factor << '\n';
    std::cout << "aggregation extreme heterogenity factor: " << aggregation_extrem_heterogenity_factor << '\n';
    std::cout << "is all node synchronized: " << (is_nodes_sync == true ? "true" : "false") << '\n';
    std::cout << "is simultaneous measurement and communication: " << (is_simultaneous_meas_and_comm == true ? "true" : "false") << "\n\n";
    std::cout << "# Measurement\n";
    std::cout << "measurement energy [μJ]: " << con_measurement_energy_mikro_J << '\n';
    std::cout << "measurement probability: " << con_measurement_prob << '\n';
    std::cout << "measurement time [msec]: " << con_measurement_time_msec << '\n';
    std::cout << "probability of extreme measurement event: " << prob_of_extreme_measurement_event << "\n";
    std::cout << "extreme measurement invalid time-delta [msec]: " << extr_meas_invalid_time_delta_msec << "\n\n";
    
    // exit(0);
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // initialize unsettable parameters
    // AoI ------------------------------------------------------------------------------
    double con_D_diagonal = sqrt(con_aoi_width_meter * con_aoi_width_meter + con_aoi_height_meter * con_aoi_height_meter);
    
    // Communication ------------------------------------------------------------------------------
    double con_frame_lengths_B = con_frame_header_length_B + con_frame_payload_sizes_B;
    // std::vector<double> con_frame_lengths_B;
    // for ( unsigned i = 0; i < con_frame_payload_sizes_B.size(); ++i ) {
    //     con_frame_lengths_B.push_back(con_frame_header_length_B + con_frame_payload_sizes_B[i]);
    // }
    double con_max_sending_range_Rmax_meter = pow(25, 1. / con_b_path_loss_exp) * con_D0_length_meter;
    // double con_max_radiant_energy_Eoutmax_mikro_J = 2500000 * 0.00001 * (5 * con_D0_length_meter) * (5 * con_D0_length_meter);
    double con_max_radiant_energy_Eoutmax_mikro_J = pow(con_max_sending_range_Rmax_meter, con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
    struct cFrame_info {
        unsigned long long time_stamp;
        double sending_time;
        double hop_count;
        double number_of_source_frames;
        double size_in_bytes;
        
        cFrame_info(unsigned long long time_stamp, double sending_time, double hop_count, double number_of_source_frames, double size_in_bytes) {
            this->time_stamp = time_stamp;
            this->sending_time = sending_time;
            this->hop_count = hop_count;
            this->number_of_source_frames = number_of_source_frames;
            this->size_in_bytes = size_in_bytes;
        }
    };
    std::vector<unsigned> successfully_received_meas_frame_per_node;
    std::vector<std::vector<unsigned>> normal_incoming_requests;
    std::vector<std::vector<unsigned>> extr_incoming_requests;
    std::vector<std::vector<cFrame_info*>> normal_input_buffers;
    std::vector<std::vector<cFrame_info*>> extr_input_buffers;
    std::vector<std::vector<std::vector<cFrame_info*>>> normal_output_buffers;
    std::vector<std::vector<std::vector<cFrame_info*>>> extr_output_buffers;
    std::vector<unsigned> output_buffer_counts;
    std::vector<std::vector<double>> distance_matrix;
    std::vector<std::vector<double>> metric_matrix;
    std::vector<std::vector<double>> prev_distances;
    std::vector<double> sending_timeouts;
    for ( unsigned j = 0; j < con_number_of_nodes + con_number_of_sinks; ++j ) {
        distance_matrix.push_back({});
        metric_matrix.push_back({});
        prev_distances.push_back({});
        for ( unsigned k = 0; k < con_number_of_nodes + con_number_of_sinks; ++k ) {
            distance_matrix[j].push_back(-1.);
            metric_matrix[j].push_back(-1.);
            prev_distances[j].push_back(-1.);
        }
    }
    std::vector<int> sending_target_by_controller;
    for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        successfully_received_meas_frame_per_node.push_back(0u);
        normal_incoming_requests.push_back({});
        extr_incoming_requests.push_back({});
        sending_timeouts.push_back(0.);
        output_buffer_counts.push_back(0u);
        normal_input_buffers.push_back({});
        normal_output_buffers.push_back({});
        extr_input_buffers.push_back({});
        extr_output_buffers.push_back({});
        for ( unsigned j = 0; j < con_number_of_nodes; ++j ) {
            normal_output_buffers[i].push_back({});
            extr_output_buffers[i].push_back({});
        }
        sending_target_by_controller.push_back(-1);
    }
    
    // General --------------------------------------------------------------------------------------------------------
    double con_alive_battery_threshold_mikro_J = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * con_D0_length_meter * con_D0_length_meter;
    double con_enhanced_node_battery_energy = con_init_battery_energy_basic_E0_mikro_J * con_enhanced_nodes_energy_multiplier;
    double con_on_to_sleep_energy_mikro_J = (con_on_mode_energy_mikro_J_per_s - con_sleep_mode_energy_mikro_J_per_s) * con_on_to_sleep_time_msec / 2000.;
    double con_sleep_to_hiber_energy_mikro_J = (con_sleep_mode_energy_mikro_J_per_s - con_hibernate_mode_energy_mikro_J_per_s) * con_sleep_to_hiber_time_msec / 2000.;
    double con_hiber_to_sleep_energy_mikro_J = (con_sleep_mode_energy_mikro_J_per_s - con_hibernate_mode_energy_mikro_J_per_s) * con_hiber_to_sleep_time_msec / 2000.;
    double con_sleep_to_on_energy_mikro_J = (con_on_mode_energy_mikro_J_per_s - con_sleep_mode_energy_mikro_J_per_s) * con_sleep_to_on_time_msec / 2000.;
    double control_traffic_energy = con_sleep_mode_energy_mikro_J_per_s + con_hibernate_mode_energy_mikro_J_per_s + (con_sleep_to_hiber_energy_mikro_J / con_sleep_to_hiber_time_msec) * 1000. + (con_hiber_to_sleep_energy_mikro_J / con_hiber_to_sleep_time_msec) * 1000.;
    // double con_sending_timeout_msec = con_hiber_to_sleep_time_msec + con_sleep_to_on_time_msec + 5;
    double con_sending_timeout_msec = 60;
    double visszajelzesi_energia = 1.2 * pow(con_D0_length_meter, con_b_path_loss_exp) * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
    // controller interval T bellitasa
    unsigned controller_interval_T_msec = (10. / con_velocity_max_km_h) * 3600.;
    // if ( is_there_sink_mobility || is_there_node_mobility ) {
    //     controller_interval_T_msec = (10. / con_velocity_max_km_h) * 3600.;
    // }
    std::vector<int> last_controller_comm_time_msec;
    for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        last_controller_comm_time_msec.push_back(-2 * controller_interval_T_msec);
    }
    
    // Simulation --------------------------------------------------------------------------------------------------
        // stop condition
    bool condition_data_received = true;
    bool is_movements_calculated = false;
    bool kiir_poziciok = false;
    unsigned long long global_time_msec = 0;
    unsigned period_time_msec = 1;
    unsigned moving_update_interval_msec = 33;
    unsigned long long last_position_update_msec = 0;
    
    // Measurement ------------------------------------------------------------------------------
    unsigned con_measurement_interval_msec = (con_sleep_to_hiber_time_msec + con_hiber_to_sleep_time_msec + sleep_time_default_msec + hiber_time_default_msec) * how_many_sleep_hibernate_cycles_in_one_T + con_on_to_sleep_time_msec + con_sleep_to_on_time_msec + on_time_default_msec + sleep_time_default_msec;
    std::vector<std::mt19937> extreme_meres_prob_gens;
    std::vector<std::mt19937> extr_meas_time_gens;
    std::vector<unsigned long long> extreme_meas_time;
    std::vector<bool> is_there_extreme_meas;
    std::vector<unsigned long long> next_measurement_time_msec;
    std::vector<long long> normal_measurement_ends_msec;
    std::vector<unsigned> number_of_normal_measurements;
    std::vector<bool> is_measurement_actually_ended;
    std::vector<std::mt19937> meas_prob_gens;
    std::vector<bool> is_there_measurement;
    std::mt19937 meastime_gen(12347);
    for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        if ( is_nodes_sync ) {
            next_measurement_time_msec.push_back(0u);
        }else {
            next_measurement_time_msec.push_back(((double)meastime_gen() / meastime_gen.max()) * con_measurement_interval_msec);
        }
        normal_measurement_ends_msec.push_back(-1234567);
        number_of_normal_measurements.push_back(0u);
        is_measurement_actually_ended.push_back(false);
        meas_prob_gens.push_back(std::mt19937(i));
        is_there_measurement.push_back(false);
        extreme_meres_prob_gens.push_back(std::mt19937(2 * i));
        extr_meas_time_gens.push_back(std::mt19937(3 * i));
        extreme_meas_time.push_back(0ull);
        is_there_extreme_meas.push_back(false);
    }
    
    // Init actual battery energies --------------------------------------------------------------
    std::mt19937 battery_gen(12346);
    std::vector<double> act_battery_energies_mikro_J;
    for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        double tmp_bat = con_init_battery_energy_basic_E0_mikro_J;
        if ( (double)battery_gen() / battery_gen.max() < con_enhanced_nodes_rate ) {
            tmp_bat *= con_enhanced_nodes_energy_multiplier;
        }
        act_battery_energies_mikro_J.push_back(tmp_bat);
    }
    
    // Statistics --------------------------------------------------------------------------------
    const unsigned number_of_out_params = 11;
    unsigned long count_of_all_measurements = 0ul;
    unsigned long count_of_all_extr_meas = 0ul;
    double act_number_of_received_frames_on_sinks = 0;
    double act_sum_of_hop_counts = 0;
    double act_sum_of_sending_times = 0;
    double relative_delivery_error = 0;
    double avg_energy_of_nodes = 100.;
    double tsum = 0.;
    for ( unsigned k = 0; k < con_number_of_nodes; ++k ) {
        tsum += act_battery_energies_mikro_J[k];
    }
    const double max_sum_energy_of_nodes = tsum;
    double avg_comm_cost = 0.;
    double sum_comm_cost = 0.;
    double alive_nodes_percent = 100.;
    double prev_elo_nodes_percent = 100.;
    
    // Init states ----------------------------------------------------------------------------------------------
    std::vector<unsigned> phases_starting_points { 0u, (unsigned)on_time_default_msec, (unsigned)(on_time_default_msec + con_on_to_sleep_time_msec) };
    for ( unsigned i = 0, k = 2; i < how_many_sleep_hibernate_cycles_in_one_T; ++i, ++k ) {
        phases_starting_points.push_back(phases_starting_points[k] + sleep_time_default_msec);
        ++k;
        phases_starting_points.push_back(phases_starting_points[k] + con_sleep_to_hiber_time_msec);
        ++k;
        phases_starting_points.push_back(phases_starting_points[k] + hiber_time_default_msec);
        ++k;
        phases_starting_points.push_back(phases_starting_points[k] + con_hiber_to_sleep_time_msec);
    }
    phases_starting_points.push_back(phases_starting_points[phases_starting_points.size() - 1] + sleep_time_default_msec);
    phases_starting_points.push_back(con_measurement_interval_msec);
    std::vector<unsigned> interval_times;
    std::vector<unsigned> in_phase_time;
    enum class extreme_enum { none, sleep_to_on, ex_on, on_to_sleep, sleep_to_hiber, sleep };
    std::vector<long> extreme_event_time_counter;
    std::vector<extreme_enum> extreme_enums;
    enum class normal_enum { on, sleep, hibernate, sleep_to_hiber, hiber_to_sleep, on_to_sleep, sleep_to_on };
    std::vector<normal_enum> normal_enums;
    std::vector<bool> is_on;
    for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        extreme_event_time_counter.push_back(0u);
        interval_times.push_back((con_measurement_interval_msec - next_measurement_time_msec[i]) % con_measurement_interval_msec);
        extreme_enums.push_back(extreme_enum::none);
        normal_enums.push_back(normal_enum::on);
        is_on.push_back(false);
        in_phase_time.push_back(0u);
    }
    
    // Write out the unsettable parameters --------------------------------------------------------------------------------------------
    std::cout << "All unsettable (by configuration file) parameters:\n";
    std::cout << "# AoI\n";
    std::cout << "Length of the diagonal of AoI: " << con_D_diagonal << "\n\n";
    std::cout << "# Communication\n";
    std::cout << "frame lengths [B]: " << con_frame_lengths_B << '\n';
    // std::cout << "frame lengths [B]: ";
    // for ( unsigned i = 0; i < con_frame_lengths_B.size(); ++i ) {
    //     std::cout << con_frame_lengths_B[i] << ", ";
    // }
    // std::cout << "\n";
    std::cout << "Max. radiant energy (Eoutmax) [μJ]: " << con_max_radiant_energy_Eoutmax_mikro_J << '\n';
    std::cout << "Max. sending range (Rmax) [m]: " << con_max_sending_range_Rmax_meter << "\n\n";
    std::cout << "# General parameters\n";
    std::cout << "Alive battery threshold [μJ]: " << con_alive_battery_threshold_mikro_J << "\n";
    std::cout << "Enhanced node battery enegy: " << con_enhanced_node_battery_energy << "\n";
    std::cout << "On-to-sleep energy [μJ]: " << con_on_to_sleep_energy_mikro_J << "\n";
    std::cout << "Sleep-to-hibernate energy [μJ]: " << con_sleep_to_hiber_energy_mikro_J << "\n";
    std::cout << "Hibernate-to-sleep energy [μJ]: " << con_hiber_to_sleep_energy_mikro_J << "\n";
    std::cout << "Sleep-to-On energy [μJ]: " << con_sleep_to_on_energy_mikro_J << "\n";
    std::cout << "Controller traffic energy [μJ]: " << control_traffic_energy << "\n";
    std::cout << "Sending timeout [msec]: " << con_sending_timeout_msec << "\n";
    std::cout << "Controller request interval (T) [msec]: " << controller_interval_T_msec << "\n\n";
    std::cout << "# Measurement\n";
    std::cout << "Measurement interval [msec]: " << con_measurement_interval_msec << "\n\n";
    std::cout << "# Initial battery energies [μJ]: \n";
    for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        std::cout << act_battery_energies_mikro_J[i] << ", ";
    }
    std::cout << "\n\n";
    std::cout << "# Mobilities\n";
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // Load mobility plugins ------------------------------------------------------------------------------
    iMobility_interface* sink_mobi_alg = nullptr;
    iMobility_interface* node_mobi_alg = nullptr;
    typedef iMobility_interface* (*Mobility_instance_fn)(void);
    void *sink_mod = nullptr;
    void *node_mod = nullptr;
        // for sink(s)
    if ( is_there_sink_mobility ) {
        strcat(sink_mobility_lib_prefix, ".so");
        sink_mod = dlopen(sink_mobility_lib_prefix, RTLD_LOCAL | RTLD_LAZY);
        if (!sink_mod) {
            std::cout << "Sink mobility plugin wasn't loaded successfully!\n";
            exit(11);
        }
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-function-type"
        Mobility_instance_fn objFunc = reinterpret_cast<Mobility_instance_fn>(dlsym(sink_mod, "getObj"));
        #pragma GCC diagnostic pop
        if ( !objFunc ) {
            std::cout << "Invalid sink mobility plugin so: 'getObj' must be defined.\n";
            exit(12);
        }
        sink_mobi_alg = objFunc();
        std::cout << "# Sink mobility algorithm name: " << sink_mobi_alg->Alg_name() << '\n';
        sink_mobi_alg->Get_client_params(&position_of_sinks_meter, &con_number_of_sinks, &position_of_nodes_meter, &con_number_of_nodes, &con_aoi_width_meter, &con_aoi_height_meter, &con_sink_diameter_meter, &con_nodes_diameter_meter, true);
        for ( unsigned long i = 0; i < sink_mobi_para_name_str_vec.size(); ++i ) {
            sink_mobi_alg->Set_parameter(sink_mobi_para_name_str_vec[i], sink_mobi_para_value_str_vec[i]);
        }
        std::cout << sink_mobi_alg->Get_params_string();
        std::cout << '\n';
        sink_mobi_alg->Init_run();
    }else {
        std::cout << "# Sink mobility: Mobility NaN\n";
    }
        // for sensornodes
    if ( is_there_node_mobility ) {
        strcat(node_mobility_lib_prefix, ".so");
        node_mod = dlopen(node_mobility_lib_prefix, RTLD_LOCAL | RTLD_LAZY);
        if (!node_mod) {
            std::cout << "Node mobility plugin wasn't loaded successfully!\n";
            exit(13);
        }
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wcast-function-type"
        Mobility_instance_fn objFunc = reinterpret_cast<Mobility_instance_fn>(dlsym(node_mod, "getObj"));
        #pragma GCC diagnostic pop
        if ( !objFunc ) {
            std::cout << "Invalid node mobility plugin so: 'getObj' must be defined.\n";
            exit(12);
        }
        node_mobi_alg = objFunc();
        std::cout << "\n# Node mobility algorithm name: " << node_mobi_alg->Alg_name() << '\n';
        node_mobi_alg->Get_client_params(&position_of_nodes_meter, &con_number_of_nodes, &position_of_sinks_meter, &con_number_of_sinks, &con_aoi_width_meter, &con_aoi_height_meter, &con_sink_diameter_meter, &con_nodes_diameter_meter, false);
        for ( unsigned long i = 0; i < node_mobi_para_name_str_vec.size(); ++i ) {
            node_mobi_alg->Set_parameter(node_mobi_para_name_str_vec[i], node_mobi_para_value_str_vec[i]);
        }
        std::cout << node_mobi_alg->Get_params_string();
        std::cout << '\n';
        node_mobi_alg->Init_run();
    }else {
        std::cout << "# Node mobility: Mobility NaN\n";
    }
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // The update matrices function ----------------------------------------------------------------------------------------
    auto Update_matrices_if_needed = [&]() {
        if ( is_movements_calculated == false ) {
            is_movements_calculated = true;
            // do mobility --------------------------------------------------------------
            unsigned move_time_range_msec = global_time_msec - last_position_update_msec;
            last_position_update_msec = global_time_msec;
            if ( move_time_range_msec > 0u ) {
                auto tmp_sipo = position_of_sinks_meter;
                if ( is_there_sink_mobility ) {
                    sink_mobi_alg->Compute_next_position(move_time_range_msec);
                }
                if ( is_there_node_mobility ) {
                    node_mobi_alg->Compute_next_position(move_time_range_msec, &tmp_sipo);
                }
            }
            // create distance and metric matrix
            for ( unsigned j = 0; j < con_number_of_nodes + con_number_of_sinks; ++j ) {
                distance_matrix[j][j] = -1.;
                metric_matrix[j][j] = -1.;
                for ( unsigned k = 0; k < j; ++k ) {
                    if ( j >= con_number_of_sinks ) {
                        unsigned node_idx_for_j = j - con_number_of_sinks;
                        if ( k >= con_number_of_sinks ) {
                            unsigned node_idx_for_k = k - con_number_of_sinks;
                            double meter_diff_x = position_of_nodes_meter[2 * node_idx_for_j] - position_of_nodes_meter[2 * node_idx_for_k];
                            double meter_diff_y = position_of_nodes_meter[2 * node_idx_for_j + 1] - position_of_nodes_meter[2 * node_idx_for_k + 1];
                            double tavolsag = sqrt(meter_diff_x * meter_diff_x + meter_diff_y * meter_diff_y);
                            if ( act_battery_energies_mikro_J[node_idx_for_j] >= con_alive_battery_threshold_mikro_J && act_battery_energies_mikro_J[node_idx_for_k] >= con_alive_battery_threshold_mikro_J && tavolsag <= con_max_sending_range_Rmax_meter ) {
                                distance_matrix[j][k] = tavolsag;
                                distance_matrix[k][j] = tavolsag;
                                if ( distance_matrix[j][k] <= con_D0_length_meter ) {
                                    tavolsag = pow(distance_matrix[j][k], 2);
                                }else {
                                    tavolsag = pow(distance_matrix[j][k], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                }
                                metric_matrix[j][k] = con_metric_alpha * (tavolsag / con_max_radiant_energy_Eoutmax_mikro_J) + (1. - con_metric_alpha) * (con_enhanced_node_battery_energy / act_battery_energies_mikro_J[node_idx_for_k]);
                                metric_matrix[k][j] = con_metric_alpha * (tavolsag / con_max_radiant_energy_Eoutmax_mikro_J) + (1. - con_metric_alpha) * (con_enhanced_node_battery_energy / act_battery_energies_mikro_J[node_idx_for_j]);
                            }else {
                                distance_matrix[j][k] = -1.;
                                metric_matrix[j][k] = -1.;
                                distance_matrix[k][j] = -1.;
                                metric_matrix[k][j] = -1.;
                            }
                        }else {
                            double meter_diff_x = position_of_nodes_meter[2 * node_idx_for_j] - position_of_sinks_meter[2 * k];
                            double meter_diff_y = position_of_nodes_meter[2 * node_idx_for_j + 1] - position_of_sinks_meter[2 * k + 1];
                            double tavolsag = sqrt(meter_diff_x * meter_diff_x + meter_diff_y * meter_diff_y);
                            if ( act_battery_energies_mikro_J[node_idx_for_j] >= con_alive_battery_threshold_mikro_J && tavolsag <= con_max_sending_range_Rmax_meter ) {
                                distance_matrix[j][k] = tavolsag;
                                distance_matrix[k][j] = tavolsag;
                                if ( distance_matrix[j][k] <= con_D0_length_meter ) {
                                    tavolsag = pow(distance_matrix[j][k], 2);
                                }else {
                                    tavolsag = pow(distance_matrix[j][k], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                }
                                metric_matrix[j][k] = con_metric_alpha * (tavolsag / con_max_radiant_energy_Eoutmax_mikro_J);
                                metric_matrix[k][j] = metric_matrix[j][k];
                            }else {
                                distance_matrix[j][k] = -1.;
                                metric_matrix[j][k] = -1.;
                                distance_matrix[k][j] = -1.;
                                metric_matrix[k][j] = -1.;
                            }
                        }
                    }else {
                        distance_matrix[j][k] = -1.;
                        metric_matrix[j][k] = -1.;
                        distance_matrix[k][j] = -1.;
                        metric_matrix[k][j] = -1.;
                    }
                }
            }
        }
    };
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // Load routing plugin ------------------------------------------------------------------------------
    iRouting_interface* routing_alg = nullptr;
    typedef iRouting_interface* (*Routing_instance_fn)(void);
    void *routing_mod = nullptr;
    strcat(routing_lib_prefix, "dijkstra_routing_plugin.so");
    routing_mod = dlopen(routing_lib_prefix, RTLD_LOCAL | RTLD_LAZY);
    if (!routing_mod) {
        std::cout << "Routing plugin wasn't loaded successfully!\n";
        exit(11);
    }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-function-type"
    Routing_instance_fn objFunc = reinterpret_cast<Routing_instance_fn>(dlsym(routing_mod, "getObj"));
    #pragma GCC diagnostic pop
    if ( !objFunc ) {
        std::cout << "Invalid routing plugin so: 'getObj' must be defined.\n";
        exit(12);
    }
    routing_alg = objFunc();
    std::cout << "# Routing algorithm name: " << routing_alg->Alg_name() << '\n';
    routing_alg->Get_client_params(&metric_matrix, con_number_of_sinks);
    // for ( unsigned long i = 0; i < sink_mobi_para_name_str_vec.size(); ++i ) {
    //     sink_mobi_alg->Set_parameter(sink_mobi_para_name_str_vec[i], sink_mobi_para_value_str_vec[i]);
    // }
    std::cout << routing_alg->Get_params_string();
    std::cout << '\n';
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // Do simulation ----------------------------------------------------------------------------------------------------
    
    // Init write data out ------------------------------------------------------------------------------------------
    
    // Open file and init
    char out_file_name[155];
    strcpy(out_file_name, common_file_name_prefix);
    strcat(out_file_name, ".out");
    std::ofstream os{out_file_name, std::ios::out | std::ios::binary};
    unsigned long long idobelyeg = 0;
    char *out_str = new char[2000000];
    unsigned long out_len = 0, vec_len = 0;
    eParams tmp_epa_enum;
    
    // Elso sor kiirasa
        // write out idobelyeg and sornak hossza
    memcpy(out_str, reinterpret_cast<char*>(&idobelyeg), sizeof(idobelyeg));
    out_len = sizeof(eParams::number_of_measurements) * number_of_out_params + sizeof(double) + sizeof(act_number_of_received_frames_on_sinks) + sizeof(double) + sizeof(double) + sizeof(relative_delivery_error) + sizeof(avg_energy_of_nodes) + sizeof(avg_comm_cost) + position_of_nodes_meter.size() * sizeof(position_of_nodes_meter[0]) + sizeof(vec_len) + position_of_sinks_meter.size() * sizeof(position_of_sinks_meter[0]) + sizeof(vec_len) + sizeof(double) + sizeof(double);
    memcpy(out_str + sizeof(idobelyeg), reinterpret_cast<char*>(&out_len), sizeof(out_len));
    out_len = sizeof(idobelyeg) + sizeof(out_len);
    
        // write out count of all measurements
    tmp_epa_enum = eParams::number_of_measurements;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::number_of_measurements));
    out_len += sizeof(eParams::number_of_measurements);
    double tmpcoam = 0.;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmpcoam), sizeof(tmpcoam));
    out_len += sizeof(tmpcoam);
    
        // write out count of all extreme measurements
    tmp_epa_enum = eParams::number_of_extreme_measurements;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::number_of_extreme_measurements));
    out_len += sizeof(eParams::number_of_extreme_measurements);
    tmpcoam = 0.;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmpcoam), sizeof(tmpcoam));
    out_len += sizeof(tmpcoam);
    
        // write out act number of received frames on sinks
    tmp_epa_enum = eParams::number_of_received_frames_on_sinks;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::number_of_received_frames_on_sinks));
    out_len += sizeof(eParams::number_of_received_frames_on_sinks);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&act_number_of_received_frames_on_sinks), sizeof(act_number_of_received_frames_on_sinks));
    out_len += sizeof(act_number_of_received_frames_on_sinks);
    
        // write out average hopcount of received frames
    tmp_epa_enum = eParams::average_hopcount_of_received_frames;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_hopcount_of_received_frames));
    double avg_hopcount_of_rec_frames = 0.;
    out_len += sizeof(eParams::average_hopcount_of_received_frames);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_hopcount_of_rec_frames), sizeof(avg_hopcount_of_rec_frames));
    out_len += sizeof(avg_hopcount_of_rec_frames);
    
        // write out average sending time of received frames
    tmp_epa_enum = eParams::average_sending_time_of_received_frames;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_sending_time_of_received_frames));
    double avg_sending_time_of_rec_frames = 0.;
    out_len += sizeof(eParams::average_sending_time_of_received_frames);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_sending_time_of_rec_frames), sizeof(avg_sending_time_of_rec_frames));
    out_len += sizeof(avg_sending_time_of_rec_frames);
    
        // write out relative delivery error
    tmp_epa_enum = eParams::relative_delivery_error;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::relative_delivery_error));
    out_len += sizeof(eParams::relative_delivery_error);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&relative_delivery_error), sizeof(relative_delivery_error));
    out_len += sizeof(relative_delivery_error);
    
        // write out average energy of nodes
    tmp_epa_enum = eParams::average_energy_of_nodes;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_energy_of_nodes));
    out_len += sizeof(eParams::average_energy_of_nodes);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_energy_of_nodes), sizeof(avg_energy_of_nodes));
    out_len += sizeof(avg_energy_of_nodes);
    
        // write out average communication energy
    tmp_epa_enum = eParams::average_comm_energy;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_comm_energy));
    out_len += sizeof(eParams::average_comm_energy);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_comm_cost), sizeof(avg_comm_cost));
    out_len += sizeof(avg_comm_cost);
    
        // write out percentage of alive nodes
    tmp_epa_enum = eParams::percentage_of_alive_nodes;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::percentage_of_alive_nodes));
    out_len += sizeof(eParams::percentage_of_alive_nodes);
    memcpy(out_str + out_len, reinterpret_cast<char*>(&alive_nodes_percent), sizeof(alive_nodes_percent));
    out_len += sizeof(alive_nodes_percent);
    
        // write out sink positions
    tmp_epa_enum = eParams::sink_positions;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::sink_positions));
    out_len += sizeof(eParams::sink_positions);
    vec_len = position_of_sinks_meter.size();
    memcpy(out_str + out_len, reinterpret_cast<char*>(&vec_len), sizeof(vec_len));
    out_len += sizeof(vec_len);
    vec_len *= sizeof(position_of_sinks_meter[0]);
    memcpy(out_str + out_len, reinterpret_cast<char*>(position_of_sinks_meter.data()), vec_len);
    out_len += vec_len;
    
        // write out node positions
    tmp_epa_enum = eParams::node_positions;
    memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::node_positions));
    out_len += sizeof(eParams::node_positions);
    vec_len = position_of_nodes_meter.size();
    memcpy(out_str + out_len, reinterpret_cast<char*>(&vec_len), sizeof(vec_len));
    out_len += sizeof(vec_len);
    vec_len *= sizeof(position_of_nodes_meter[0]);
    memcpy(out_str + out_len, reinterpret_cast<char*>(position_of_nodes_meter.data()), vec_len);
    out_len += vec_len;
    
        // write out the first row
    os.write(out_str, out_len);
    
    // The core of the simulation ------------------------------------------------------------------------------------------------------------------
    // debug variables
    int debug_nem_erte_el_count = 0, debug_idotullepes_count = 0, debug_idotul_keretek_szama = 0;
    // --debug variables
    while ( true ) {
        // regular movement update
        if ( global_time_msec % moving_update_interval_msec == 0 ) {
            unsigned mozgasi_ido_msec = global_time_msec - last_position_update_msec;
            last_position_update_msec = global_time_msec;
            if ( mozgasi_ido_msec > 0u ) {
                auto tmp_sipo = position_of_sinks_meter;
                if ( is_there_sink_mobility ) {
                    sink_mobi_alg->Compute_next_position(mozgasi_ido_msec);
                }
                if ( is_there_node_mobility ) {
                    node_mobi_alg->Compute_next_position(mozgasi_ido_msec, &tmp_sipo);
                }
            }
        }
        
        // init to write data out
        bool changed_variables[number_of_out_params];
        for ( unsigned i = 0; i < number_of_out_params; ++i ) {
            changed_variables[i] = false;
        }
        
        // handling states
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            // generate next extreme event
            if ( interval_times[i] == 0 ) {
                extreme_meas_time[i] = ((double)extreme_meres_prob_gens[i]() / extreme_meres_prob_gens[i].max() < prob_of_extreme_measurement_event ? ((double)extr_meas_time_gens[i]() / extr_meas_time_gens[i].max()) * con_measurement_interval_msec : 2 * con_measurement_interval_msec);
            }
            
            // get normal state
            unsigned which_phase = 0;
            for ( unsigned j = 0; j < phases_starting_points.size(); ++j ) {
                if ( phases_starting_points[j] > interval_times[i] ) {
                    in_phase_time[i] = interval_times[i] - phases_starting_points[j - 1];
                    break;
                }
                ++which_phase;
            }
            normal_enum tenum = normal_enum::sleep;
            if ( which_phase == 1 ) {
                tenum = normal_enum::on;
            }else if ( which_phase == 2 ) {
                tenum = normal_enum::on_to_sleep;
            }else if ( which_phase == phases_starting_points.size() - 1 ) {
                tenum = normal_enum::sleep_to_on;
            }else if ( (which_phase - 3) % 4 == 0 ) {
                tenum = normal_enum::sleep;
            }else if ( which_phase % 4 == 0 ) {
                tenum = normal_enum::sleep_to_hiber;
            }else if ( (which_phase - 1) % 4 == 0 ) {
                tenum = normal_enum::hibernate;
            }else if ( (which_phase - 2) % 4 == 0 ) {
                tenum = normal_enum::hiber_to_sleep;
            }
            normal_enums[i] = tenum;
            
            // handle extreme state
            if ( extreme_enums[i] == extreme_enum::sleep_to_on && extreme_event_time_counter[i] == con_sleep_to_on_time_msec ) {
                extreme_enums[i] = extreme_enum::ex_on;
                extreme_event_time_counter[i] = 0;
            }else if ( extreme_enums[i] == extreme_enum::on_to_sleep && extreme_event_time_counter[i] == con_on_to_sleep_time_msec ) {
                if ( tenum != normal_enum::hibernate && tenum != normal_enum::hiber_to_sleep ) {
                    extreme_enums[i] = extreme_enum::none;
                }else {
                    unsigned xcount = (interval_times[i] + (unsigned)con_sleep_to_hiber_time_msec) % con_measurement_interval_msec;
                    which_phase = 0;
                    for ( unsigned j = 0; j < phases_starting_points.size(); ++j ) {
                        if ( phases_starting_points[j] > xcount ) {
                            break;
                        }
                        ++which_phase;
                    }
                    // if the state is hibernate or hiber-to-sleep
                    if ( (which_phase - 1) % 4 == 0 || (which_phase - 2) % 4 == 0 ) {
                        extreme_enums[i] = extreme_enum::sleep_to_hiber;
                        extreme_event_time_counter[i] = 0;
                    }else {
                        extreme_enums[i] = extreme_enum::sleep;
                    }
                }
            }else if ( extreme_enums[i] == extreme_enum::sleep_to_hiber && extreme_event_time_counter[i] == con_sleep_to_hiber_time_msec ) {
                extreme_enums[i] = extreme_enum::none;
            }else if ( extreme_enums[i] == extreme_enum::sleep && tenum == normal_enum::sleep ) {
                extreme_enums[i] = extreme_enum::none;
            }
            // increase extreme event time counter
            ++extreme_event_time_counter[i];
        }
        
        is_movements_calculated = false;
        // forwarding loop
        alive_nodes_percent = 0.;
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            // debug
            // if ( i == 54 ) {
            //     std::cout << "time:" << global_time_msec << ", n: ";
            //     if ( normal_enums[i] == normal_enum::on ) {
            //         std::cout << "on";
            //     }else if ( normal_enums[i] == normal_enum::sleep ) {
            //         std::cout << "sleep";
            //     }else if ( normal_enums[i] == normal_enum::hibernate ) {
            //         std::cout << "hibernate";
            //     }else if ( normal_enums[i] == normal_enum::sleep_to_hiber ) {
            //         std::cout << "sleep_to_hiber";
            //     }else if ( normal_enums[i] == normal_enum::hiber_to_sleep ) {
            //         std::cout << "hiber_to_sleep";
            //     }else if ( normal_enums[i] == normal_enum::on_to_sleep ) {
            //         std::cout << "on_to_sleep";
            //     }else if ( normal_enums[i] == normal_enum::sleep_to_on ) {
            //         std::cout << "sleep_to_on";
            //     }
            //     std::cout << ", ex: ";
            //     if ( extreme_enums[i] == extreme_enum::none ) {
            //         std::cout << "none";
            //     }else if ( extreme_enums[i] == extreme_enum::sleep_to_on ) {
            //         std::cout << "sleep_to_on";
            //     }else if ( extreme_enums[i] == extreme_enum::ex_on ) {
            //         std::cout << "ex_on";
            //     }else if ( extreme_enums[i] == extreme_enum::on_to_sleep ) {
            //         std::cout << "on_to_sleep";
            //     }else if ( extreme_enums[i] == extreme_enum::sleep_to_hiber ) {
            //         std::cout << "sleep_to_hiber";
            //     }else if ( extreme_enums[i] == extreme_enum::sleep ) {
            //         std::cout << "sleep";
            //     }
            //     std::cout << '\n';
            // }
            // --debug
            // if is alive then do things
            is_on[i] = false;
            if ( act_battery_energies_mikro_J[i] >= con_alive_battery_threshold_mikro_J ) {
                // different acts based on the actual normal and extreme states
                // if ( normal_enums[i] == normal_enum::on || extreme_enums[i] == extreme_enum::ex_on || (output_buffer_counts[i] > 0 && sending_target_by_controller[i] != -1) ) {
                if ( normal_enums[i] == normal_enum::on || extreme_enums[i] == extreme_enum::ex_on || output_buffer_counts[i] > 0 ) {
                    is_on[i] = true;
                    // extreme measurement case
                    int celpont_id = -1;
                    // creating frame
                    cFrame_info* tfi = new cFrame_info(global_time_msec, 0., 1., 0., 0.);
                    bool target_already_computed = false;
                    int number_of_bytes = 0;
                    if ( interval_times[i] >= extreme_meas_time[i] ) {
                        extreme_meas_time[i] += con_measurement_interval_msec;
                        is_there_extreme_meas[i] = true;
                        act_battery_energies_mikro_J[i] -= con_measurement_energy_mikro_J;
                    }
                    if ( is_there_extreme_meas[i] ) {
                        is_there_extreme_meas[i] = false;
                        if ( global_time_msec - normal_measurement_ends_msec[i] > extr_meas_invalid_time_delta_msec ) {
                            ++count_of_all_measurements;
                            ++count_of_all_extr_meas;
                            changed_variables[(int)eParams::number_of_measurements] = true;
                            changed_variables[(int)eParams::number_of_extreme_measurements] = true;
                            tfi->number_of_source_frames = 1.;
                        }
                    }
                    for ( unsigned j = 0; j < extr_input_buffers[i].size(); ++j ) {
                        tfi->sending_time += extr_input_buffers[i][j]->sending_time;
                        tfi->hop_count += extr_input_buffers[i][j]->hop_count;
                        tfi->number_of_source_frames += extr_input_buffers[i][j]->number_of_source_frames;
                    }
                    number_of_bytes = std::round(con_frame_payload_sizes_B * tfi->number_of_source_frames);
                    int how_many_frames = 0;
                    if ( number_of_bytes > 0 ) {
                        target_already_computed = true;
                        // choose where to send
                        Update_matrices_if_needed();
                        if ( global_time_msec - last_controller_comm_time_msec[i] >= controller_interval_T_msec ) {
                            last_controller_comm_time_msec[i] = global_time_msec;
                            celpont_id = routing_alg->Where_to_send(i + con_number_of_sinks);
                            sending_target_by_controller[i] = celpont_id;
                            // subtract controll comm. energy
                            act_battery_energies_mikro_J[i] -= control_traffic_energy;
                            sum_comm_cost += control_traffic_energy;
                            // set prev distance
                            if ( celpont_id != -1 ) {
                                prev_distances[i + con_number_of_sinks][celpont_id] = distance_matrix[i + con_number_of_sinks][celpont_id];
                            }
                        }else {
                            celpont_id = sending_target_by_controller[i];
                        }
                        // do aggregation
                        number_of_bytes = std::round(number_of_bytes * aggregation_extrem_heterogenity_factor);
                        // prepare tfi frames for sending
                        how_many_frames = number_of_bytes / (int)con_frame_payload_sizes_B;
                        if ( number_of_bytes % (int)con_frame_payload_sizes_B != 0 ) {
                            ++how_many_frames;
                        }
                        tfi->sending_time /= (double)how_many_frames;
                        tfi->hop_count /= (double)how_many_frames;
                        tfi->number_of_source_frames /= (double)how_many_frames;
                        // debug
                        // if ( i == 11 ) {
                        //     std::cout << "yeah, " << celpont_id << '\n';
                        // }
                        // --debug
                    }
                    
                    if ( celpont_id != -1 && act_battery_energies_mikro_J[i] >= how_many_frames * con_alive_battery_threshold_mikro_J ) {
                        int kcount = 0;
                        while ( number_of_bytes > 0 ) {
                            number_of_bytes -= con_frame_payload_sizes_B;
                            // subtract aggr. energy
                            if ( prev_distances[i + con_number_of_sinks][celpont_id] <= con_D0_length_meter ) {
                                act_battery_energies_mikro_J[i] -= 0.1 * con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], 2);
                            }else {
                                act_battery_energies_mikro_J[i] -= 0.1 * con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                            }
                            // clone the first frame
                            if ( kcount > 0 ) {
                                tfi = new cFrame_info(tfi->time_stamp, tfi->sending_time, tfi->hop_count, tfi->number_of_source_frames, 0.);
                            }
                            // initiate sending
                            if ( celpont_id < (int)con_number_of_sinks ) {
                                if ( distance_matrix[i + con_number_of_sinks][celpont_id] <= prev_distances[i + con_number_of_sinks][celpont_id] ) {
                                    condition_data_received = true;
                                    // statistics
                                    act_number_of_received_frames_on_sinks += tfi->number_of_source_frames;
                                    act_sum_of_hop_counts += tfi->hop_count;
                                    act_sum_of_sending_times += tfi->sending_time + 1. / (double)how_many_frames;
                                    changed_variables[(int)eParams::number_of_received_frames_on_sinks] = true;
                                    changed_variables[(int)eParams::average_hopcount_of_received_frames] = true;
                                    changed_variables[(int)eParams::average_sending_time_of_received_frames] = true;
                                }
                                // debug
                                else {
                                    debug_nem_erte_el_count += tfi->number_of_source_frames * how_many_frames;
                                    std::cout << "nem erte el count (ex sink): " << debug_nem_erte_el_count << '\n';
                                }
                                // --debug
                                // subtract sending energy
                                if ( prev_distances[i + con_number_of_sinks][celpont_id] <= con_D0_length_meter ) {
                                    double kien = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], 2);
                                    act_battery_energies_mikro_J[i] -= kien;
                                    sum_comm_cost += kien;
                                }else {
                                    double kien = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                    act_battery_energies_mikro_J[i] -= kien;
                                    sum_comm_cost += kien;
                                }
                                // ex state
                                // if ( output_buffer_counts[i] == 0 && extreme_enums[i] == extreme_enum::ex_on ) {
                                //     extreme_enums[i] = extreme_enum::on_to_sleep;
                                //     extreme_event_time_counter[i] = 0;
                                // }
                            }else {
                                if ( distance_matrix[i + con_number_of_sinks][celpont_id] <= prev_distances[i + con_number_of_sinks][celpont_id] ) {
                                    extr_incoming_requests[celpont_id - con_number_of_sinks].push_back(i);
                                }
                                // debug
                                else {
                                    debug_nem_erte_el_count += tfi->number_of_source_frames * how_many_frames;
                                    std::cout << "nem erte el count (ex node): " << debug_nem_erte_el_count << '\n';
                                }
                                // --debug
                                extr_output_buffers[i][celpont_id - con_number_of_sinks].push_back(tfi);
                                ++output_buffer_counts[i];
                                sending_timeouts[i] = 0.;
                            }
                            // subtract jelzo jel energy
                            if ( kcount == 0 ) {
                                double kien = 1.2 * pow(prev_distances[i + con_number_of_sinks][celpont_id], con_b_path_loss_exp) * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                act_battery_energies_mikro_J[i] -= kien;
                                sum_comm_cost += kien;
                            }
                            ++kcount;
                        }
                        extr_input_buffers[i].clear();
                    }else {
                        // if ( extreme_enums[i] == extreme_enum::ex_on ) {
                        //     extreme_enums[i] = extreme_enum::on_to_sleep;
                        //     extreme_event_time_counter[i] = 0;
                        // }
                        // debug
                        if ( number_of_bytes > 0 ) {
                            std::cout << "cel neg: " << celpont_id << "\n";
                        }
                        // --debug
                    }
                    
                    // szokasos meres
                    if ( normal_enums[i] == normal_enum::on || output_buffer_counts[i] > 0 ) {
                        tfi = new cFrame_info(global_time_msec, 0., 1., 0., 0.);
                        number_of_bytes = 0;
                        if ( normal_enums[i] == normal_enum::on && in_phase_time[i] == 0 && (double)meas_prob_gens[i]() / meas_prob_gens[i].max() < con_measurement_prob && act_battery_energies_mikro_J[i] > con_measurement_energy_mikro_J ) {
                            is_there_measurement[i] = true;
                        }
                        if ( is_there_measurement[i] ) {
                            if ( in_phase_time[i] < con_measurement_time_msec ) {
                                act_battery_energies_mikro_J[i] -= con_measurement_energy_mikro_J / con_measurement_time_msec;
                            }else if ( in_phase_time[i] == con_measurement_time_msec ) {
                                is_there_measurement[i] = false;
                                normal_measurement_ends_msec[i] = global_time_msec;
                                ++number_of_normal_measurements[i];
                                ++count_of_all_measurements;
                                changed_variables[(int)eParams::number_of_measurements] = true;
                                tfi->number_of_source_frames = 1.;
                            }
                        }
                        for ( unsigned j = 0; j < normal_input_buffers[i].size(); ++j ) {
                            tfi->sending_time += normal_input_buffers[i][j]->sending_time;
                            tfi->hop_count += normal_input_buffers[i][j]->hop_count;
                            tfi->number_of_source_frames += normal_input_buffers[i][j]->number_of_source_frames;
                        }
                        number_of_bytes = std::round(con_frame_payload_sizes_B * tfi->number_of_source_frames);
                        if ( number_of_bytes > 0 ) {
                            // choose where to send
                            Update_matrices_if_needed();
                            if ( target_already_computed == false ) {
                                if ( global_time_msec - last_controller_comm_time_msec[i] >= controller_interval_T_msec ) {
                                    last_controller_comm_time_msec[i] = global_time_msec;
                                    celpont_id = routing_alg->Where_to_send(i + con_number_of_sinks);
                                    sending_target_by_controller[i] = celpont_id;
                                    // subtract controll comm. energy
                                    act_battery_energies_mikro_J[i] -= control_traffic_energy;
                                    sum_comm_cost += control_traffic_energy;
                                    // set prev distance
                                    if ( celpont_id != -1 ) {
                                        prev_distances[i + con_number_of_sinks][celpont_id] = distance_matrix[i + con_number_of_sinks][celpont_id];
                                    }
                                }else {
                                    celpont_id = sending_target_by_controller[i];
                                }
                            }
                            // do aggregation
                            number_of_bytes = std::round(number_of_bytes * aggregation_heterogenity_factor);
                            // prepare tfi frames for sending
                            how_many_frames = number_of_bytes / (int)con_frame_payload_sizes_B;
                            if ( number_of_bytes % (int)con_frame_payload_sizes_B != 0 ) {
                                ++how_many_frames;
                            }
                            tfi->sending_time /= (double)how_many_frames;
                            tfi->hop_count /= (double)how_many_frames;
                            tfi->number_of_source_frames /= (double)how_many_frames;
                            // debug
                            // if ( i == 11 ) {
                            //     std::cout << "yeah2, " << celpont_id << '\n';
                            // }
                            // if ( global_time_msec == 405 ) {
                            //     std::cout << "mure, " << celpont_id << '\n';
                            // }
                            // --debug
                        }
                        if ( number_of_bytes > 0 && celpont_id != -1 && act_battery_energies_mikro_J[i] >= how_many_frames * con_alive_battery_threshold_mikro_J ) {
                            int kcount = 0;
                            while ( number_of_bytes > 0 ) {
                                number_of_bytes -= con_frame_payload_sizes_B;
                                // subtract aggr. energy
                                if ( prev_distances[i + con_number_of_sinks][celpont_id] <= con_D0_length_meter ) {
                                    act_battery_energies_mikro_J[i] -= 0.1 * con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], 2);
                                }else {
                                    act_battery_energies_mikro_J[i] -= 0.1 * con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                }
                                // clone the first frame
                                if ( kcount > 0 ) {
                                    tfi = new cFrame_info(tfi->time_stamp, tfi->sending_time, tfi->hop_count, tfi->number_of_source_frames, 0.);
                                }
                                // initiate sending
                                if ( celpont_id < (int)con_number_of_sinks ) {
                                    if ( distance_matrix[i + con_number_of_sinks][celpont_id] <= prev_distances[i + con_number_of_sinks][celpont_id] ) {
                                        condition_data_received = true;
                                        // statistics
                                        act_number_of_received_frames_on_sinks += tfi->number_of_source_frames;
                                        act_sum_of_hop_counts += tfi->hop_count;
                                        act_sum_of_sending_times += tfi->sending_time + 1. / (double)how_many_frames;
                                        changed_variables[(int)eParams::number_of_received_frames_on_sinks] = true;
                                        changed_variables[(int)eParams::average_hopcount_of_received_frames] = true;
                                        changed_variables[(int)eParams::average_sending_time_of_received_frames] = true;
                                        // debug
                                        // if ( global_time_msec == 1120 ) {
                                        //     std::cout << "uszkvemore, hc: " << tfi->hop_count << ", sf: " << tfi->number_of_source_frames << "\n";
                                        // }
                                        // --debug
                                    }
                                    // debug
                                    else {
                                        debug_nem_erte_el_count += tfi->number_of_source_frames * how_many_frames;
                                        std::cout << "nem erte el count (no sink): " << debug_nem_erte_el_count << '\n';
                                    }
                                    // --debug
                                    // subtract sending energy
                                    if ( prev_distances[i + con_number_of_sinks][celpont_id] <= con_D0_length_meter ) {
                                        double kien = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], 2);
                                        act_battery_energies_mikro_J[i] -= kien;
                                        sum_comm_cost += kien;
                                    }else {
                                        double kien = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[i + con_number_of_sinks][celpont_id], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                        act_battery_energies_mikro_J[i] -= kien;
                                        sum_comm_cost += kien;
                                    }
                                }else {
                                    if ( distance_matrix[i + con_number_of_sinks][celpont_id] <= prev_distances[i + con_number_of_sinks][celpont_id] ) {
                                        normal_incoming_requests[celpont_id - con_number_of_sinks].push_back(i);
                                    }
                                    // debug
                                    else {
                                        debug_nem_erte_el_count += tfi->number_of_source_frames * how_many_frames;
                                        std::cout << "nem erte el count (no node): " << debug_nem_erte_el_count << '\n';
                                    }
                                    // --debug
                                    normal_output_buffers[i][celpont_id - con_number_of_sinks].push_back(tfi);
                                    ++output_buffer_counts[i];
                                    sending_timeouts[i] = 0.;
                                }
                                // subtract jelzo jel energy
                                if ( kcount == 0 ) {
                                    double kien = 1.2 * pow(prev_distances[i + con_number_of_sinks][celpont_id], con_b_path_loss_exp) * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                                    act_battery_energies_mikro_J[i] -= kien;
                                    sum_comm_cost += kien;
                                }
                                ++kcount;
                            }
                            normal_input_buffers[i].clear();
                        }else {
                            // if ( extreme_enums[i] == extreme_enum::ex_on ) {
                            //     extreme_enums[i] = extreme_enum::on_to_sleep;
                            //     extreme_event_time_counter[i] = 0;
                            // }
                            // debug
                            if ( number_of_bytes > 0 ) {
                                std::cout << "cel neg: " << celpont_id << "\n";
                            }
                            // --debug
                        }
                    }
                    
                    // on state energy subtract
                    act_battery_energies_mikro_J[i] -= con_on_mode_energy_mikro_J_per_s / 1000.;
                // sleep state
                }else if ( (normal_enums[i] == normal_enum::sleep && extreme_enums[i] == extreme_enum::none) || extreme_enums[i] == extreme_enum::sleep ) {
                    // felgyult esemenyek kezelese
                    // if extreme meas. or if incoming request(s)
                    if ( interval_times[i] >= extreme_meas_time[i] ) {
                        extreme_meas_time[i] += con_measurement_interval_msec;
                        is_there_extreme_meas[i] = true;
                        act_battery_energies_mikro_J[i] -= con_measurement_energy_mikro_J;
                        extreme_enums[i] = extreme_enum::sleep_to_on;
                        extreme_event_time_counter[i] = 1;
                        act_battery_energies_mikro_J[i] -= con_sleep_to_on_energy_mikro_J / con_sleep_to_on_time_msec;
                    }else if ( extr_incoming_requests[i].size() > 0 ) {
                        extreme_enums[i] = extreme_enum::sleep_to_on;
                        extreme_event_time_counter[i] = 1;
                        act_battery_energies_mikro_J[i] -= con_sleep_to_on_energy_mikro_J / con_sleep_to_on_time_msec;
                    }else {
                        // energy subtract
                        act_battery_energies_mikro_J[i] -= con_sleep_mode_energy_mikro_J_per_s / 1000.;
                    }
                }else if ( (normal_enums[i] == normal_enum::on_to_sleep && extreme_enums[i] == extreme_enum::none) || extreme_enums[i] == extreme_enum::on_to_sleep ) {
                    act_battery_energies_mikro_J[i] -= con_on_to_sleep_energy_mikro_J / con_on_to_sleep_time_msec;
                }else if ( (normal_enums[i] == normal_enum::sleep_to_on && extreme_enums[i] == extreme_enum::none) || extreme_enums[i] == extreme_enum::sleep_to_on ) {
                    act_battery_energies_mikro_J[i] -= con_sleep_to_on_energy_mikro_J / con_sleep_to_on_time_msec;
                }else if ( normal_enums[i] == normal_enum::sleep_to_hiber || extreme_enums[i] == extreme_enum::sleep_to_hiber ) {
                    act_battery_energies_mikro_J[i] -= con_sleep_to_hiber_energy_mikro_J / con_sleep_to_hiber_time_msec;
                }else if ( normal_enums[i] == normal_enum::hibernate ) {
                    act_battery_energies_mikro_J[i] -= con_hibernate_mode_energy_mikro_J_per_s / 1000.;
                }else if ( normal_enums[i] == normal_enum::hiber_to_sleep ) {
                    act_battery_energies_mikro_J[i] -= con_hiber_to_sleep_energy_mikro_J / con_hiber_to_sleep_time_msec;
                }
            }
        }
        
        // receiving loops
        std::vector<bool> is_just_output_buffer_empty;
        std::vector<bool> is_extreme_fogadott;
        std::vector<bool> is_normal_fogadott;
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            is_just_output_buffer_empty.push_back(false);
            is_extreme_fogadott.push_back(false);
            is_normal_fogadott.push_back(false);
        }
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            if ( is_on[i] ) {
                std::vector<unsigned> inre_seids;
                // extreme incoming requests
                for ( unsigned j = 0; j < extr_incoming_requests[i].size(); ++j ) {
                    // debug
                    // if ( global_time_msec == 185515 ) {
                    //     std::cout << "a fene enne meg: " << '\n';
                    // }
                    // --debug
                    condition_data_received = true;
                    Update_matrices_if_needed();
                    unsigned sender_node_id = extr_incoming_requests[i][j];
                    bool is_nincs_id = true;
                    for ( unsigned k = 0; k < inre_seids.size(); ++k ) {
                        if ( inre_seids[k] == sender_node_id ) {
                            is_nincs_id = false;
                            break;
                        }
                    }
                    if ( is_nincs_id ) {
                        inre_seids.push_back(sender_node_id);
                    }
                    // extreme keret fogadas
                    for ( unsigned k = 0; k < extr_output_buffers[sender_node_id][i].size(); ++k ) {
                        extr_output_buffers[sender_node_id][i][k]->sending_time += global_time_msec - extr_output_buffers[sender_node_id][i][k]->time_stamp + 1;
                        double kuldesi_energia;
                        // sending energy meghat.
                        if ( prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] <= con_D0_length_meter ) {
                            kuldesi_energia = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks], 2);
                        }else {
                            kuldesi_energia = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                        }
                        // accept frame sending and receiving if there are enough battery energies on sender and receiver
                        if ( act_battery_energies_mikro_J[sender_node_id] >= kuldesi_energia ) {
                            // fogadasi energia meghat.
                            double fogadasi_energia = con_frame_receive_energy_Eelec_mikro_J_per_bit * con_frame_lengths_B * 8.;
                            if ( distance_matrix[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] <= prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] && act_battery_energies_mikro_J[i] >= fogadasi_energia ) {
                                // fogadas
                                extr_input_buffers[i].push_back(extr_output_buffers[sender_node_id][i][k]);
                                // fogadasi energia levonasa
                                act_battery_energies_mikro_J[i] -= fogadasi_energia;
                                sum_comm_cost += fogadasi_energia;
                                is_extreme_fogadott[i] = true;
                            }else {
                                // fogadas
                                // extr_input_buffers[sender_node_id].push_back(extr_output_buffers[sender_node_id][i][k]);
                                // debug
                                ++debug_nem_erte_el_count;
                                std::cout << "nem erte el count (ex fog): " << debug_nem_erte_el_count << '\n';
                                // --debug
                            }
                            // subtract sending energy
                            act_battery_energies_mikro_J[sender_node_id] -= kuldesi_energia;
                            sum_comm_cost += kuldesi_energia;
                        }else {
                            // fogadas
                            extr_input_buffers[sender_node_id].push_back(extr_output_buffers[sender_node_id][i][k]);
                        }
                        // output buffer stuff
                        --output_buffer_counts[sender_node_id];
                        if ( output_buffer_counts[sender_node_id] == 0 ) {
                            is_just_output_buffer_empty[sender_node_id] = true;
                        }
                    }
                    extr_output_buffers[sender_node_id][i].clear();
                }
                extr_incoming_requests[i].clear();
                // normal incoming requests
                for ( unsigned j = 0; j < normal_incoming_requests[i].size(); ++j ) {
                    condition_data_received = true;
                    Update_matrices_if_needed();
                    unsigned sender_node_id = normal_incoming_requests[i][j];
                    bool is_nincs_id = true;
                    for ( unsigned k = 0; k < inre_seids.size(); ++k ) {
                        if ( inre_seids[k] == sender_node_id ) {
                            is_nincs_id = false;
                            break;
                        }
                    }
                    if ( is_nincs_id ) {
                        inre_seids.push_back(sender_node_id);
                    }
                    // normal keret fogadas
                    for ( unsigned k = 0; k < normal_output_buffers[sender_node_id][i].size(); ++k ) {
                        normal_output_buffers[sender_node_id][i][k]->sending_time += global_time_msec - normal_output_buffers[sender_node_id][i][k]->time_stamp + 1;
                        double kuldesi_energia;
                        // sending energy meghat.
                        if ( prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] <= con_D0_length_meter ) {
                            kuldesi_energia = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks], 2);
                        }else {
                            kuldesi_energia = con_frame_lengths_B * 8 * con_energyfree_spacefactor_Efs_mikro_J_per_bit_per_square_meter * pow(prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks], con_b_path_loss_exp) / pow(con_D0_length_meter, con_b_path_loss_exp - 2);
                        }
                        // accept frame sending and receiving if there are enough battery energies on sender and receiver
                        if ( act_battery_energies_mikro_J[sender_node_id] >= kuldesi_energia ) {
                            // fogadasi energia meghat.
                            double fogadasi_energia = con_frame_receive_energy_Eelec_mikro_J_per_bit * con_frame_lengths_B * 8.;
                            if ( distance_matrix[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] <= prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] && act_battery_energies_mikro_J[i] >= fogadasi_energia ) {
                                // fogadas
                                normal_input_buffers[i].push_back(normal_output_buffers[sender_node_id][i][k]);
                                // fogadasi energia levonasa
                                act_battery_energies_mikro_J[i] -= fogadasi_energia;
                                sum_comm_cost += fogadasi_energia;
                                is_normal_fogadott[i] = true;
                            }else {
                                // fogadas
                                // normal_input_buffers[sender_node_id].push_back(normal_output_buffers[sender_node_id][i][k]);
                                // debug
                                ++debug_nem_erte_el_count;
                                std::cout << "nem erte el count (norm fog): " << debug_nem_erte_el_count << ", prev: " << prev_distances[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] << ", norm: " << distance_matrix[sender_node_id + con_number_of_sinks][i + con_number_of_sinks] << ", energy: " << act_battery_energies_mikro_J[i] << ", fog en: " << fogadasi_energia << ", glob time: " << global_time_msec << '\n';
                                // --debug
                            }
                            // subtract sending energy
                            act_battery_energies_mikro_J[sender_node_id] -= kuldesi_energia;
                            sum_comm_cost += kuldesi_energia;
                        }else {
                            // fogadas
                            normal_input_buffers[sender_node_id].push_back(normal_output_buffers[sender_node_id][i][k]);
                        }
                        // output buffer stuff
                        --output_buffer_counts[sender_node_id];
                        if ( output_buffer_counts[sender_node_id] == 0 ) {
                            is_just_output_buffer_empty[sender_node_id] = true;
                        }
                    }
                    normal_output_buffers[sender_node_id][i].clear();
                }
                normal_incoming_requests[i].clear();
                // visszajelzes energia koltseg levonasa
                act_battery_energies_mikro_J[i] -= visszajelzesi_energia * inre_seids.size();
                sum_comm_cost += visszajelzesi_energia * inre_seids.size();
            }
        }
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            // turn off extreme state? also with expired timeouts
            if ( extr_input_buffers[i].size() == 0 && normal_input_buffers[i].size() == 0 ) {
                if ( output_buffer_counts[i] == 0 && (extreme_enums[i] == extreme_enum::ex_on || (is_on[i] && normal_enums[i] != normal_enum::on)) ) {
                    extreme_enums[i] = extreme_enum::on_to_sleep;
                    extreme_event_time_counter[i] = 0;
                }
                // if ( is_normal_fogadott[i] && !is_extreme_fogadott[i] && extreme_enums[i] == extreme_enum::ex_on && output_buffer_counts[i] == 0 ) {
                //     extreme_enums[i] = extreme_enum::on_to_sleep;
                //     extreme_event_time_counter[i] = 0;
                // }else if ( is_just_output_buffer_empty[i] && (extreme_enums[i] == extreme_enum::ex_on || normal_enums[i] != normal_enum::on) ) {
                //     extreme_enums[i] = extreme_enum::on_to_sleep;
                //     extreme_event_time_counter[i] = 0;
                // }
                if ( output_buffer_counts[i] > 0 ) {
                    ++sending_timeouts[i];
                }
                if ( sending_timeouts[i] > con_sending_timeout_msec && is_on[i] && normal_enums[i] != normal_enum::on ) {
                    for ( unsigned j = 0; j < con_number_of_nodes; ++j ) {
                        // debug
                        debug_idotul_keretek_szama += extr_output_buffers[i][j].size();
                        debug_idotul_keretek_szama += normal_output_buffers[i][j].size();
                        // --debug
                        extr_output_buffers[i][j].clear();
                        normal_output_buffers[i][j].clear();
                    }
                    sending_timeouts[i] = 0;
                    output_buffer_counts[i] = 0;
                    extreme_enums[i] = extreme_enum::on_to_sleep;
                    extreme_event_time_counter[i] = 0;
                    // debug
                    ++debug_idotullepes_count;
                    std::cout << "timeout happened: " << debug_idotullepes_count << ", keretek: " << debug_idotul_keretek_szama << "\n";
                    // --debug
                }
            }
            
            // increase in-interval times
            interval_times[i] = (interval_times[i] + 1) % con_measurement_interval_msec;
        }
        
        // stat number of alive nodes
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            if ( act_battery_energies_mikro_J[i] >= con_alive_battery_threshold_mikro_J ) {
                ++alive_nodes_percent;
            }
        }
        alive_nodes_percent /= con_number_of_nodes;
        alive_nodes_percent *= 100.;
        if ( alive_nodes_percent != prev_elo_nodes_percent ) {
            prev_elo_nodes_percent = alive_nodes_percent;
            changed_variables[(int)eParams::percentage_of_alive_nodes] = true;
        }
        
        // debug prints
        // if ( global_time_msec % 10000 == 0 ) {
        //     std::cout << global_time_msec / 1000 << " s\n" << std::flush;
        // }
        // if ( global_time_msec == 600 ) {
        //     std::cout << "\n";
        //     for ( unsigned i = 0; i < con_number_of_sinks; ++i ) {
        //         std::cout << position_of_sinks_meter[i * 2] << ", ";
        //         std::cout << position_of_sinks_meter[i * 2 + 1] << ", ";
        //     }
        //     std::cout << "\n";
        //     for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
        //         std::cout << position_of_nodes_meter[i * 2] << ", ";
        //         std::cout << position_of_nodes_meter[i * 2 + 1] << ", ";
        //     }
        //     std::cout << "\n";
        // }
        
        // write out data -----------------------------------------------------------------------------------------------------------
        if ( global_time_msec != 0 ) {
                // checking loop
            bool is_ch_param = false;
            out_len = 0;
            for ( unsigned i = 0; i < number_of_out_params; ++i ) {
                if ( changed_variables[i] == true ) {
                    is_ch_param = true;
                    eParams z_enum = (eParams)i;
                    if ( z_enum == eParams::number_of_measurements ) {
                        out_len += sizeof(double) + sizeof(eParams::number_of_measurements);
                    }else if ( z_enum == eParams::number_of_extreme_measurements ) {
                        out_len += sizeof(double) + sizeof(eParams::number_of_extreme_measurements);
                    }else if ( z_enum == eParams::number_of_received_frames_on_sinks ) {
                        out_len += sizeof(act_number_of_received_frames_on_sinks) + sizeof(eParams::number_of_received_frames_on_sinks);
                    }else if ( z_enum == eParams::average_hopcount_of_received_frames ) {
                        out_len += sizeof(avg_hopcount_of_rec_frames) + sizeof(eParams::average_hopcount_of_received_frames);
                    }else if ( z_enum == eParams::average_sending_time_of_received_frames ) {
                        out_len += sizeof(avg_sending_time_of_rec_frames) + sizeof(eParams::average_sending_time_of_received_frames);
                    }else if ( z_enum == eParams::percentage_of_alive_nodes ) {
                        out_len += sizeof(alive_nodes_percent) + sizeof(eParams::percentage_of_alive_nodes);
                    }
                }
            }
            if ( is_ch_param ) {
                out_len += sizeof(relative_delivery_error) + sizeof(eParams::relative_delivery_error);
            }
            memcpy(out_str, reinterpret_cast<char*>(&global_time_msec), sizeof(global_time_msec));
            out_len += sizeof(avg_energy_of_nodes) + sizeof(eParams::average_energy_of_nodes);
            out_len += sizeof(avg_comm_cost) + sizeof(eParams::average_comm_energy);
            if ( kiir_poziciok ) {
                out_len += position_of_sinks_meter.size() * sizeof(position_of_sinks_meter[0]) + sizeof(eParams::sink_positions) + sizeof(vec_len);
                out_len += position_of_nodes_meter.size() * sizeof(position_of_nodes_meter[0]) + sizeof(eParams::node_positions) + sizeof(vec_len);
            }
            memcpy(out_str + sizeof(global_time_msec), reinterpret_cast<char*>(&out_len), sizeof(out_len));
            out_len = sizeof(global_time_msec) + sizeof(out_len);
            
                // writer loop
            for ( unsigned i = 0; i < number_of_out_params; ++i ) {
                if ( changed_variables[i] == true ) {
                    eParams z_enum = (eParams)i;
                    if ( z_enum == eParams::number_of_measurements ) {
                        tmp_epa_enum = eParams::number_of_measurements;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::number_of_measurements));
                        out_len += sizeof(eParams::number_of_measurements);
                        double leosztva = (double)count_of_all_measurements / con_number_of_nodes;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&leosztva), sizeof(leosztva));
                        out_len += sizeof(leosztva);
                    }else if ( z_enum == eParams::number_of_extreme_measurements ) {
                        tmp_epa_enum = eParams::number_of_extreme_measurements;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::number_of_extreme_measurements));
                        out_len += sizeof(eParams::number_of_extreme_measurements);
                        double leosztva = (double)count_of_all_extr_meas / con_number_of_nodes;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&leosztva), sizeof(leosztva));
                        out_len += sizeof(leosztva);
                    }else if ( z_enum == eParams::number_of_received_frames_on_sinks ) {
                        tmp_epa_enum = eParams::number_of_received_frames_on_sinks;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::number_of_received_frames_on_sinks));
                        out_len += sizeof(eParams::number_of_received_frames_on_sinks);
                        double leosztva = act_number_of_received_frames_on_sinks / con_number_of_nodes;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&leosztva), sizeof(leosztva));
                        out_len += sizeof(leosztva);
                    }else if ( z_enum == eParams::average_hopcount_of_received_frames ) {
                        tmp_epa_enum = eParams::average_hopcount_of_received_frames;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_hopcount_of_received_frames));
                        double avg_hopcount_of_rec_frames = (double)act_sum_of_hop_counts / act_number_of_received_frames_on_sinks;
                        out_len += sizeof(eParams::average_hopcount_of_received_frames);
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_hopcount_of_rec_frames), sizeof(avg_hopcount_of_rec_frames));
                        out_len += sizeof(avg_hopcount_of_rec_frames);
                    }else if ( z_enum == eParams::average_sending_time_of_received_frames ) {
                        tmp_epa_enum = eParams::average_sending_time_of_received_frames;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_sending_time_of_received_frames));
                        double avg_sending_time_of_rec_frames = (double)act_sum_of_sending_times / act_number_of_received_frames_on_sinks;
                        out_len += sizeof(eParams::average_sending_time_of_received_frames);
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_sending_time_of_rec_frames), sizeof(avg_sending_time_of_rec_frames));
                        out_len += sizeof(avg_sending_time_of_rec_frames);
                    }else if ( z_enum == eParams::percentage_of_alive_nodes ) {
                        tmp_epa_enum = eParams::percentage_of_alive_nodes;
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::percentage_of_alive_nodes));
                        out_len += sizeof(eParams::percentage_of_alive_nodes);
                        memcpy(out_str + out_len, reinterpret_cast<char*>(&alive_nodes_percent), sizeof(alive_nodes_percent));
                        out_len += sizeof(alive_nodes_percent);
                    }
                }
            }
            if ( is_ch_param ) {
                    // write out relative delivery error
                tmp_epa_enum = eParams::relative_delivery_error;
                memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::relative_delivery_error));
                out_len += sizeof(eParams::relative_delivery_error);
                relative_delivery_error = ((double)(count_of_all_measurements - act_number_of_received_frames_on_sinks) / count_of_all_measurements) * 100.;
                memcpy(out_str + out_len, reinterpret_cast<char*>(&relative_delivery_error), sizeof(relative_delivery_error));
                out_len += sizeof(relative_delivery_error);
            }
                
                // write out average energy of nodes
            tmp_epa_enum = eParams::average_energy_of_nodes;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_energy_of_nodes));
            out_len += sizeof(eParams::average_energy_of_nodes);
            double tsum = 0.;
            for ( unsigned k = 0; k < con_number_of_nodes; ++k ) {
                tsum += act_battery_energies_mikro_J[k];
            }
            avg_energy_of_nodes = (tsum / max_sum_energy_of_nodes) * 100.;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_energy_of_nodes), sizeof(avg_energy_of_nodes));
            out_len += sizeof(avg_energy_of_nodes);
            
                // write out average comm. energy
            tmp_epa_enum = eParams::average_comm_energy;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::average_comm_energy));
            out_len += sizeof(eParams::average_comm_energy);
            avg_comm_cost = (sum_comm_cost / max_sum_energy_of_nodes) * 100.;
            memcpy(out_str + out_len, reinterpret_cast<char*>(&avg_comm_cost), sizeof(avg_comm_cost));
            out_len += sizeof(avg_comm_cost);
            
            if ( kiir_poziciok ) {
                    // write out sink positions
                tmp_epa_enum = eParams::sink_positions;
                memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::sink_positions));
                out_len += sizeof(eParams::sink_positions);
                vec_len = position_of_sinks_meter.size();
                memcpy(out_str + out_len, reinterpret_cast<char*>(&vec_len), sizeof(vec_len));
                out_len += sizeof(vec_len);
                vec_len *= sizeof(position_of_sinks_meter[0]);
                memcpy(out_str + out_len, reinterpret_cast<char*>(position_of_sinks_meter.data()), vec_len);
                out_len += vec_len;
                
                    // write out node positions
                tmp_epa_enum = eParams::node_positions;
                memcpy(out_str + out_len, reinterpret_cast<char*>(&tmp_epa_enum), sizeof(eParams::node_positions));
                out_len += sizeof(eParams::node_positions);
                vec_len = position_of_nodes_meter.size();
                memcpy(out_str + out_len, reinterpret_cast<char*>(&vec_len), sizeof(vec_len));
                out_len += sizeof(vec_len);
                vec_len *= sizeof(position_of_nodes_meter[0]);
                memcpy(out_str + out_len, reinterpret_cast<char*>(position_of_nodes_meter.data()), vec_len);
                out_len += vec_len;
            }
            
            // write out row
            os.write(out_str, out_len);
        }
        
        // stop condition
        // if ( global_time_msec % con_measurement_interval_msec == 0 ) {
        //     if ( condition_data_received ) {
        //         condition_data_received = false;
        //     }else {
        //         break;
        //     }
        // }
        bool is_vege = true;
        for ( unsigned i = 0; i < con_number_of_nodes; ++i ) {
            if ( act_battery_energies_mikro_J[i] >= con_alive_battery_threshold_mikro_J ) {
                is_vege = false;
                break;
            }
        }
        if ( is_vege ) {
            break;
        }
        
        // increase global time
        global_time_msec += period_time_msec;
    }
    
    // Write out closing row ---------------------------------------------------------------------------------------------------------------------------
        // write out idobelyeg and sorhossz ami itt 0
    memcpy(out_str, reinterpret_cast<char*>(&global_time_msec), sizeof(global_time_msec));
    out_len = 0;
    memcpy(out_str + sizeof(global_time_msec), reinterpret_cast<char*>(&out_len), sizeof(out_len));
        // utolso sor kiirasa
    out_len = sizeof(global_time_msec) + sizeof(out_len);
    os.write(out_str, out_len);
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // Clean up --------------------------------------------------------------------------------------------------------------------------
    os.close();
    if ( sink_mobi_alg ) {
        delete sink_mobi_alg;
    }
    if ( node_mobi_alg ) {
        delete node_mobi_alg;
    }
    if ( routing_alg ) {
        delete routing_alg;
    }
    if ( sink_mod ) {
        dlclose(sink_mod);
    }
    if ( node_mod ) {
        dlclose(node_mod);
    }
    if ( routing_mod ) {
        dlclose(routing_mod);
    }
    
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    // kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
    
    // do csv generation ----------------------------------------------------------------------------------------------------------------
    // debug
    std::cout << "csv generalas\n" << std::flush;
    // --debug
    char csv_gen_command[500];
    strcpy(csv_gen_command, "./csv_gen ");
    strcat(csv_gen_command, out_file_name);
    strcat(csv_gen_command, " 100 number_of_measurements number_of_received_frames_on_sinks average_hopcount_of_received_frames relative_delivery_error average_energy_of_nodes average_comm_energy number_of_extreme_measurements percentage_of_alive_nodes");
    system(csv_gen_command);
}