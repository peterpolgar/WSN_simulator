#ifndef MOBILITY_INTERFACE_H
#define MOBILITY_INTERFACE_H

#include <string>

class iMobility_interface
{
public:
    virtual ~iMobility_interface() = default;
    virtual std::string Alg_name() = 0;
    virtual void Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
                                   unsigned *number_of_get_items, double *width, double *height, double *sink_radius, double *node_radius, bool is_for_sinks_not_nodes) = 0;
    virtual void Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec = {}) = 0;
    virtual void Set_parameter(std::string param_str, std::string ertek_str) = 0;
    virtual iMobility_interface* Create_new_instance() = 0;
    virtual void Init_run() = 0;
    virtual std::string Get_params_string() = 0;

    std::vector<double> *set_pos_vec;
    unsigned *number_of_set_items;
    const std::vector<double> *get_pos_vec;
    unsigned *number_of_get_items;
    double *area_width;
    double *area_height;
    double *sink_radius;
    double *node_radius;
    bool is_for_sinks_not_nodes;
    double col_avoid_radius_basic;
    enum class eCollision_avoidance { none, opposite_dir };
    eCollision_avoidance collision_avoidance;
    double distance_from_aoi_edges = 0.;
    unsigned speed_range_start;
    unsigned speed_range_end;
};

#endif // MOBILITY_INTERFACE_H
