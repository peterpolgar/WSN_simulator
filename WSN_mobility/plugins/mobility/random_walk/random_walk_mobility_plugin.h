#ifndef RANDOM_WALK_MOBILITY_PLUGIN_H
#define RANDOM_WALK_MOBILITY_PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <random>
#include <cmath>
#include "mobility_interface.h"

class cRandom_walk_mobility : public QObject, iMobility_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mobility.interface" FILE "random_walk.json")
    Q_INTERFACES(iMobility_interface)

public:
    cRandom_walk_mobility();
    QString Alg_name() override;
    bool Is_for_sinks() override;
    bool Is_for_nodes() override;
    void Get_client_params(std::vector<double> *set_pos_vec, int *number_of_set_items, const std::vector<double> *get_pos_vec,
                           int *number_of_get_items, int *sink_radius, int *node_radius, int *meter_width, int *meter_height, bool is_for_sinks_not_nodes) override;
    void Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec) override;
    void Set_parameter(std::string param_str, std::string ertek_str) override;
    std::vector<sParam>* Get_parameters() override;
    void Set_parameter(int param_id, double value_or_index) override;
    iMobility_interface* Create_new_instance() override;
    QString Get_params_string() override;
    void Init_run() override;
    void Unit_len_changed() override;
    void Resize_happened(double new_old_rate) override;
    void Get_widgets(std::vector<QWidget*> widgets) override;

    bool Is_dist_lesser_than_rad(int a, int b);
    bool Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec);

    std::uint_fast32_t seed;
    unsigned time_period_msecs;
    double dir_range_start;
    double dir_range_end;
    unsigned speed_range_start;
    unsigned speed_range_end;
    bool is_speed_uniform_not_normal;
    double speed_normal_mean;
    double speed_normal_std_dev;
    bool is_global_not_local_cs;
    double col_avoid_radius_square;

    double dir_range_len;
    unsigned speed_range_len;
    unsigned cum_time;
    std::vector<bool> is_in_coll_rad;

    std::vector<std::mt19937> dir_gens;
    std::vector<std::mt19937> speed_gens;
    std::normal_distribution<> normal_dist;
    std::function<double(int)> Speed_gen_fn;

    std::vector<double> act_dirs;
    std::vector<double> act_speeds;
    std::vector<double> double_positions;
    const double two_pi = 2 * M_PI;
    const double pi_per_180 = M_PI / 180;
};

#endif // RANDOM_WALK_MOBILITY_PLUGIN_H
