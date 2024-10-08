#ifndef GAUSS_MARKOV_MOBILITY_PLUGIN_H
#define GAUSS_MARKOV_MOBILITY_PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <random>
#include <cmath>
#include "mobility_interface.h"

class cGauss_markov_mobility : public QObject, iMobility_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mobility.interface" FILE "gauss_markov.json")
    Q_INTERFACES(iMobility_interface)

public:
    cGauss_markov_mobility();
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
    double dir_mean;
    double dir_normal_mean;
    double dir_normal_std_dev;
    unsigned speed_mean;
    double speed_normal_mean;
    double speed_normal_std_dev;
    double alfa;

    double col_avoid_radius_square;
    std::vector<bool> is_in_coll_rad;
    unsigned cum_time;

    std::vector<std::mt19937> speed_gens;
    std::vector<std::mt19937> dir_gens;
    std::normal_distribution<> speed_normal_dist;
    std::normal_distribution<> dir_normal_dist;
    std::function<double(int)> Speed_gen_fn;
    std::function<double(int)> Dir_gen_fn;
    const double speed_max = 400.;

    std::vector<double> act_dirs;
    std::vector<double> act_speeds;
    std::vector<double> act_dir_means;
    std::vector<double> double_positions;
    const double two_pi = 2 * M_PI;
    const double pi_per_180 = M_PI / 180;
    const double pi_per_2 = M_PI / 2.;
    const double pi_per_4 = M_PI / 4.;
    const double fok_135_in_rad = pi_per_4 * 3;
    const double fok_225_in_rad = M_PI + pi_per_4;
    const double fok_270_in_rad = two_pi - pi_per_2;
    const double fok_315_in_rad = two_pi - pi_per_4;
};

#endif // GAUSS_MARKOV_MOBILITY_PLUGIN_H
