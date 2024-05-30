#ifndef ROUTING_INTERFACE_H
#define ROUTING_INTERFACE_H

#include <string>

class iRouting_interface
{
public:
    virtual ~iRouting_interface() = default;
    virtual std::string Alg_name() = 0;
    virtual void Get_client_params(std::vector<std::vector<double>>* metric_matrix, unsigned number_of_sinks) = 0;
    virtual int Where_to_send(int idx) = 0;
    virtual void Set_parameter(std::string param_str, std::string ertek_str) = 0;
    virtual iRouting_interface* Create_new_instance() = 0;
    virtual void Init_run() = 0;
    virtual std::string Get_params_string() = 0;

    std::vector<std::vector<double>>* metric_matrix;
    unsigned number_of_sinks;
};

#endif // ROUTING_INTERFACE_H
