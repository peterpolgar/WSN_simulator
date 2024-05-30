#ifndef DIRECTED_UNIQUE_DIRECTION_MOBILITY_PLUGIN_H
#define DIRECTED_UNIQUE_DIRECTION_MOBILITY_PLUGIN_H

#include <random>
#include <cmath>
#include "../mobility_interface.h"

class cDirected_unique_direction_mobility : public iMobility_interface
{

public:
    cDirected_unique_direction_mobility();
    std::string Alg_name() override;
    void Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
                                   unsigned *number_of_get_items, double *width, double *height, double *sink_radius, double *node_radius, bool is_for_sinks_not_nodes) override;
    void Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec) override;
    void Set_parameter(std::string param_str, std::string ertek_str) override;
    iMobility_interface* Create_new_instance() override;
    void Init_run() override;
    std::string Get_params_string() override;

    bool Is_dist_lesser_than_rad(int a, int b);
    bool Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec);

    std::uint_fast32_t seed;
    unsigned speed;

    double col_avoid_radius_square;
    std::vector<bool> is_in_coll_rad;

    std::mt19937 gen;
    std::vector<double> act_dirs;
    double the_dir;
    double the_opposite_dir;
    const double two_pi = 2 * M_PI;
};

#endif // DIRECTED_UNIQUE_DIRECTION_MOBILITY_PLUGIN_H
