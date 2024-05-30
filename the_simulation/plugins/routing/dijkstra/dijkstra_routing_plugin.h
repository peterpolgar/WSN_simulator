#ifndef DIJKSTRA_ROUTING_PLUGIN_H
#define DIJKSTRA_ROUTING_PLUGIN_H

#include <random>
#include <cmath>
#include <functional>
#include <memory>
#include "../routing_interface.h"

class cDijkstra_routing : public iRouting_interface
{

public:
    cDijkstra_routing();
    std::string Alg_name() override;
    void Get_client_params(std::vector<std::vector<double>>* metric_matrix, unsigned number_of_sinks) override;
    int Where_to_send(int idx) override;
    void Set_parameter(std::string param_str, std::string ertek_str) override;
    iRouting_interface* Create_new_instance() override;
    void Init_run() override;
    std::string Get_params_string() override;
};

#endif // DIJKSTRA_ROUTING_PLUGIN_H
