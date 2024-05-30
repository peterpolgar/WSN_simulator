#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <dlfcn.h>
#include "plugins/mobility/mobility_interface.h"

int main(int argc, char const *argv[])
{
    std::vector<double> position_of_sinks_meter{ 5.836478,11.476510,70.440252,46.006711,89.056604,7.550336,20.628931,57.785235,133.735849,72.483221 };
    unsigned con_number_of_sinks = 5;
    std::vector<double> position_of_nodes_meter{ 5.836478,11.476510,32.201258,25.570470,39.748428,9.865772,5.534591,45.805369,57.761006,16.208054,38.540881,43.288591,55.547170,48.624161,11.572327,64.630872,41.660377,68.154362,32.503145,86.073826,71.849057,65.134228,13.383648,85.872483,86.943396,34.932886,66.113208,0.604027,110.088050,20.134228,14.088050,27.885906,57.962264,81.241611,98.918239,6.342282,105.056604,39.463087,124.477987,4.228188,75.572327,20.536913,101.333333,59.395973,134.540881,52.651007,68.930818,37.348993,154.264151,49.228188,126.188679,27.382550,140.075472,75.000000,78.792453,82.147651,139.974843,14.496644,158.893082,21.342282 };
    unsigned con_number_of_nodes = 30;
    double con_aoi_width_meter = 160;
    double con_aoi_height_meter = 90;
    double con_sink_diameter_meter = 0.2;
    double con_nodes_diameter_meter = 0.1;
    std::vector<std::string> sink_mobi_para_name_str_vec{ "seed" };
    std::vector<std::string> sink_mobi_para_value_str_vec{ "12" };
    //  -  -  -  -  -  -  -  -  - 
    if ( argc < 2 ) {
        std::cerr << "Plugin name must be given. Exit.\n";
        exit(1);
    }
    char sink_mobility_lib_prefix[80];
    strcpy(sink_mobility_lib_prefix, "./plugins/mobility/");
    strcat(sink_mobility_lib_prefix, argv[1]);
    iMobility_interface* sink_mobi_alg = nullptr;
    typedef iMobility_interface* (*Mobility_instance_fn)(void);
    void *sink_mod = nullptr;
    double velocity_max_km_h = 50.;
    bool is_there_sink_mobility = true;
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
        velocity_max_km_h = sink_mobi_alg->speed_range_end;
        sink_mobi_alg->Init_run();
        // sink_mobi_alg->Compute_next_position(1990);
        for ( int i = 0; i < 60; ++i ) {
            sink_mobi_alg->Compute_next_position(33);
        }
        std::cout << '\n';
        for ( unsigned i = 0; i < con_number_of_sinks; ++i ) {
            std::cout << position_of_sinks_meter[i * 2] << ", ";
            std::cout << position_of_sinks_meter[i * 2 + 1] << ", ";
        }
        std::cout << '\n';
    }else {
        std::cout << "# Sink mobility: Mobility NaN\n";
    }
}