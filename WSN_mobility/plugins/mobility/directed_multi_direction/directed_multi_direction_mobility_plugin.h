#ifndef DIRECTED_MULTI_DIRECTION_MOBILITY_PLUGIN_H
#define DIRECTED_MULTI_DIRECTION_MOBILITY_PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <random>
#include <cmath>
#include "mobility_interface.h"

class cDirected_multi_direction_mobility : public QObject, iMobility_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mobility.interface" FILE "directed_multi_direction.json")
    Q_INTERFACES(iMobility_interface)

public:
    cDirected_multi_direction_mobility();
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
    unsigned speed;

    double col_avoid_radius_square;
    std::vector<bool> is_in_coll_rad;

    std::vector<std::mt19937> gens;
    std::vector<double> act_dirs;
    std::vector<double> the_dirs;
    std::vector<double> the_opposite_dirs;
    std::vector<double> double_positions;
    const double two_pi = 2 * M_PI;
};

#endif // DIRECTED_MULTI_DIRECTION_MOBILITY_PLUGIN_H
