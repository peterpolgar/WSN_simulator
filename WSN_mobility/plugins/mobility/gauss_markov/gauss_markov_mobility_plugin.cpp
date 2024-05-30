#include "gauss_markov_mobility_plugin.h"
#include <cslider.h>
#include <cfp_slider.h>
#include <QComboBox>
#include <QSlider>

cGauss_markov_mobility::cGauss_markov_mobility()
{
    seed = 0;
    time_period_msecs = 1000;
    dir_mean = M_PI;
    dir_normal_mean = 0.;
    dir_normal_std_dev = pi_per_2;
    dir_normal_dist.param(std::normal_distribution<>::param_type(dir_normal_mean, dir_normal_std_dev));
    Dir_gen_fn = [this](int i){
        double a = dir_normal_dist(dir_gens[i]);
        a *= dir_normal_std_dev * sqrt(1 - alfa * alfa);
        a += (1 - alfa) * act_dir_means[i];
        a += alfa * act_dirs[i];
        return a >= two_pi ? std::fmod(a, two_pi) : a < 0. ? fabs(std::fmod(a + two_pi, two_pi)) : a;
    };
    speed_mean = 40;
    speed_normal_mean = 0.;
    speed_normal_std_dev = 10.;
    speed_normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
    Speed_gen_fn = [this](int i){
        double a = speed_normal_dist(speed_gens[i]);
        a *= speed_normal_std_dev * sqrt(1 - alfa * alfa);
        a += (1 - alfa) * speed_mean;
        a += alfa * act_speeds[i];
        return a > speed_max ? std::fmod(a, speed_max) : a < 0. ? std::fmod(-a, speed_max) : a;
    };
    alfa = 0.5;
    distance_from_aoi_edges_m = 5.;

    collision_avoidance = none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

QString cGauss_markov_mobility::Alg_name()
{
    return "Gauss-Markov";
}

bool cGauss_markov_mobility::Is_for_sinks()
{
    return true;
}

bool cGauss_markov_mobility::Is_for_nodes()
{
    return true;
}

void cGauss_markov_mobility::Get_client_params(std::vector<double> *set_pos_vec, int *number_of_set_items, const std::vector<double> *get_pos_vec,
                                              int *number_of_get_items, int *sink_radius, int *node_radius, int *meter_width, int *meter_height, bool is_for_sinks_not_nodes)
{
    this->set_pos_vec = set_pos_vec;
    this->number_of_set_items = number_of_set_items;
    this->get_pos_vec = get_pos_vec;
    this->number_of_get_items = number_of_get_items;
    this->sink_radius = sink_radius;
    this->node_radius = node_radius;
    this->area_m_width = meter_width;
    this->area_m_height = meter_height;
    this->is_for_sinks_not_nodes = is_for_sinks_not_nodes;
}

void cGauss_markov_mobility::Get_widgets(std::vector<QWidget*> widgets)
{
    this->widgets = widgets;
    this->widgets.at(2 * 11)->hide();
    this->widgets.at(2 * 11 + 1)->hide();
}

void cGauss_markov_mobility::Unit_len_changed()
{
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
//    distance_from_aoi_edges = (distance_from_aoi_edges_m / *area_m_width) * *area_m_width;
}

void cGauss_markov_mobility::Resize_happened(double new_old_rate)
{
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
//    for ( int i = 0; i < *number_of_set_items; ++i ) {
//        double_positions.at(i * 2) *= new_old_rate;
//        double_positions.at(i * 2 + 1) *= new_old_rate;
//    }
//    distance_from_aoi_edges = (distance_from_aoi_edges_m / *area_m_width) * *area_m_width;
}

bool cGauss_markov_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = double_positions.at(a * 2) - double_positions.at(b * 2);
    double delta_y = double_positions.at(a * 2 + 1) - double_positions.at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cGauss_markov_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = double_positions.at(a * 2) - get_vec->at(b * 2);
    double delta_y = double_positions.at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cGauss_markov_mobility::Init_run()
{
    cum_time = 0;
    act_dirs.clear();
    act_speeds.clear();
    act_dir_means.clear();
    double_positions.clear();
    is_in_coll_rad.clear();
    dir_gens.clear();
    speed_gens.clear();
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        dir_gens.push_back(std::mt19937(seed + i));
        speed_gens.push_back(std::mt19937(seed + i + 1234567));
        act_dirs.push_back(((double) dir_gens[i]() / dir_gens[i].max()) * two_pi);
        act_speeds.push_back(((double) speed_gens[i]() / speed_gens[i].max()) * 50 + 1);
        double_positions.push_back(set_pos_vec->at(i * 2));
        double_positions.push_back(set_pos_vec->at(i * 2 + 1));
        if ( double_positions[i * 2] < distance_from_aoi_edges_m ) {
            if ( double_positions[i * 2 + 1] < distance_from_aoi_edges_m ) {
                act_dir_means.push_back(pi_per_4);
            }else if ( double_positions[i * 2 + 1] > *area_m_height - distance_from_aoi_edges_m ) {
                act_dir_means.push_back(fok_315_in_rad);
            }else {
                act_dir_means.push_back(0.);
            }
        }else if ( double_positions[i * 2] > *area_m_width - distance_from_aoi_edges_m ) {
            if ( double_positions[i * 2 + 1] < distance_from_aoi_edges_m ) {
                act_dir_means.push_back(fok_135_in_rad);
            }else if ( double_positions[i * 2 + 1] > *area_m_height - distance_from_aoi_edges_m ) {
                act_dir_means.push_back(fok_225_in_rad);
            }else {
                act_dir_means.push_back(M_PI);
            }
        }else if ( double_positions[i * 2 + 1] < distance_from_aoi_edges_m ) {
            act_dir_means.push_back(pi_per_2);
        }else if ( double_positions[i * 2 + 1] > *area_m_height - distance_from_aoi_edges_m ) {
            act_dir_means.push_back(fok_270_in_rad);
        }else {
            act_dir_means.push_back(dir_mean);
        }
        act_dirs[i] = Dir_gen_fn(i);
        is_in_coll_rad.push_back(false);
    }
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        if ( ! is_in_coll_rad.at(i) ) {
            for ( int j = 0; j < *number_of_set_items; ++j ) {
                if ( j != i && Is_dist_lesser_than_rad(i, j) ) {
                    is_in_coll_rad.at(i) = true;
                    is_in_coll_rad.at(j) = true;
                    break;
                }
            }
        }
    }
}

void cGauss_markov_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
{
    if ( collision_avoidance == opposite_dir ) {
        const std::vector<double> *tmp_tmp_get_vec = (get_tmp_pos_vec == nullptr ? get_pos_vec : get_tmp_pos_vec);
        const int civ = *number_of_get_items;
        for ( int i = 0; i < *number_of_set_items; ++i ) {
            bool tmpb = false;
            for ( int j = 0; j < *number_of_set_items; ++j ) {
                if ( j != i && Is_dist_lesser_than_rad(i, j) ) {
                    tmpb = true;
                    if ( ! is_in_coll_rad.at(i) ) {
                        act_dirs.at(i) += M_PI;
                        if ( act_dirs.at(i) > two_pi ) {
                            act_dirs.at(i) -= two_pi;
                        }
                    }
                    break;
                }
            }
            if ( tmpb == false ) {
                for ( int j = 0; j < civ; ++j ) {
                    if ( Is_dist_lesser_than_rad_with_get(i, j, tmp_tmp_get_vec) ) {
                        tmpb = true;
                        if ( ! is_in_coll_rad.at(i) ) {
                            act_dirs.at(i) += M_PI;
                            if ( act_dirs.at(i) > two_pi ) {
                                act_dirs.at(i) -= two_pi;
                            }
                        }
                        break;
                    }
                }
            }
            is_in_coll_rad.at(i) = tmpb;
        }
    }

    cum_time += time_in_msec;
    int diff = time_in_msec;
    if ( cum_time > time_period_msecs ) {
        diff = cum_time - time_period_msecs;
        if ( diff > time_in_msec ) { diff = time_in_msec; }
        double size_rate = (double)(time_in_msec - diff) / 3600;
        for ( int i = 0; i < *number_of_set_items; ++i ) {
            double vt = size_rate * act_speeds[i];
            double tmpx = double_positions.at(i * 2) + vt * cos(act_dirs[i]);
            bool x_not_changed = true;
            if ( tmpx <= *area_m_width ) {
                if ( tmpx < 0 ) {
                    x_not_changed = false;
                }
            }else {
                x_not_changed = false;
            }
            if ( x_not_changed ) {
                double tmpy = double_positions.at(i * 2 + 1) + vt * sin(act_dirs[i]);
                if ( tmpy <= *area_m_height ) {
                    if ( tmpy >= 0 ) {
                        double_positions.at(i * 2 + 1) = tmpy;
                        double_positions.at(i * 2) = tmpx;
                    }
                }
            }
            act_speeds.at(i) = Speed_gen_fn(i);
            if ( double_positions[i * 2] < distance_from_aoi_edges_m ) {
                if ( double_positions[i * 2 + 1] < distance_from_aoi_edges_m ) {
                    act_dir_means[i] = pi_per_4;
                }else if ( double_positions[i * 2 + 1] > *area_m_height - distance_from_aoi_edges_m ) {
                    act_dir_means[i] = fok_315_in_rad;
                }else {
                    act_dir_means[i] = 0.;
                }
            }else if ( double_positions[i * 2] > *area_m_width - distance_from_aoi_edges_m ) {
                if ( double_positions[i * 2 + 1] < distance_from_aoi_edges_m ) {
                    act_dir_means[i] = fok_135_in_rad;
                }else if ( double_positions[i * 2 + 1] > *area_m_height - distance_from_aoi_edges_m ) {
                    act_dir_means[i] = fok_225_in_rad;
                }else {
                    act_dir_means[i] = M_PI;
                }
            }else if ( double_positions[i * 2 + 1] < distance_from_aoi_edges_m ) {
                act_dir_means[i] = pi_per_2;
            }else if ( double_positions[i * 2 + 1] > *area_m_height - distance_from_aoi_edges_m ) {
                act_dir_means[i] = fok_270_in_rad;
            }
            act_dirs[i] = Dir_gen_fn(i);
        }
        cum_time = diff;
    }

    double size_rate = (double)diff / 3600;
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        double vt = size_rate * act_speeds[i];
        double tmpx = double_positions.at(i * 2) + vt * cos(act_dirs[i]);
        bool x_not_changed = true;
        if ( tmpx <= *area_m_width ) {
            if ( tmpx < 0 ) {
                x_not_changed = false;
            }
        }else {
            x_not_changed = false;
        }
        if ( x_not_changed ) {
            double tmpy = double_positions.at(i * 2 + 1) + vt * sin(act_dirs[i]);
            if ( tmpy <= *area_m_height ) {
                if ( tmpy >= 0 ) {
                    double_positions.at(i * 2 + 1) = tmpy;
                    double_positions.at(i * 2) = tmpx;
                }
            }
        }
        set_pos_vec->at(i * 2) = double_positions.at(i * 2);
        set_pos_vec->at(i * 2 + 1) = double_positions.at(i * 2 + 1);
    }
}

void cGauss_markov_mobility::Set_parameter(std::string param_str, std::string ertek_str)
{
    bool is_number = true;
    double value_or_index = 0;
    try
    {
        std::stod(ertek_str);
    }
    catch (std::invalid_argument const& ex)
    {
        is_number = false;
    }
    if ( is_number ) {
        value_or_index = std::stod(ertek_str);
        if ( param_str == "seed" ) {
            reinterpret_cast<cSlider*>(widgets[1])->setValue(value_or_index);
        }else if ( param_str == "time period length [s]" ) {
            reinterpret_cast<cSlider*>(widgets[3])->setValue(value_or_index);
        }else if ( param_str == "alpha" ) {
            reinterpret_cast<cFp_slider*>(widgets[5])->Set_fp_value(value_or_index);
        }else if ( param_str == "distance from AoI edges [m]" ) {
            reinterpret_cast<cFp_slider*>(widgets[7])->Set_fp_value(value_or_index);
        }else if ( param_str == "speed mean [km/h]" ) {
            reinterpret_cast<cSlider*>(widgets[9])->setValue(value_or_index);
        }else if ( param_str == "speed Gaussian mean [km/h]" ) {
            reinterpret_cast<cFp_slider*>(widgets[11])->Set_fp_value(value_or_index);
        }else if ( param_str == "speed Gaussian std dev [km/h]" ) {
            reinterpret_cast<cFp_slider*>(widgets[13])->Set_fp_value(value_or_index);
        }else if ( param_str == "direction mean [degree]" ) {
            reinterpret_cast<cSlider*>(widgets[15])->setValue(value_or_index);
        }else if ( param_str == "direction Gaussian mean [degree]" ) {
            reinterpret_cast<cFp_slider*>(widgets[17])->Set_fp_value(value_or_index);
        }else if ( param_str == "direction Gaussian std dev [degree]" ) {
            reinterpret_cast<cFp_slider*>(widgets[19])->Set_fp_value(value_or_index);
        }else if ( param_str == "collision avoidance radius [cm]" ) {
            reinterpret_cast<cSlider*>(widgets[23])->setValue(value_or_index);
        }
    }else {
        if ( ertek_str[ertek_str.size() - 1] == '\r' ) {
            ertek_str.resize(ertek_str.size() - 1);
        }
        if ( param_str == "collision avoidance" ) {
            int tmp = (ertek_str.compare("opposite direction") == 0 ? 1 : 0);
            reinterpret_cast<QComboBox*>(widgets[21])->setCurrentIndex(tmp);
        }
    }
}

std::vector<cGauss_markov_mobility::sParam>* cGauss_markov_mobility::Get_parameters()
{
    auto vec = new std::vector<cGauss_markov_mobility::sParam>();
    vec->push_back(sParam {0, "Seed", false, true, 0, 1000, 0, nullptr, 0});
    vec->push_back(sParam {1, "Time period length [s]", false, true, 1, 120, 1, nullptr, 0});
    vec->push_back(sParam {2, "Alpha", false, false, 0, 1, 0.5, nullptr, 0});
    vec->push_back(sParam {3, "Distance from AoI edges [m]", false, false, 0, 30, 5, nullptr, 0});
    vec->push_back(sParam {4, "Speed mean [km/h]", false, true, 0, 200, 40, nullptr, 0});
    vec->push_back(sParam {5, "Speed Gaussian mean [km/h]", false, false, 0, 100, 0, nullptr, 0});
    vec->push_back(sParam {6, "Speed Gaussian std dev [km/h]", false, false, 1, 100, 10, nullptr, 0});
    vec->push_back(sParam {7, "Direction mean [degree]", false, true, 0, 359, 180, nullptr, 0});
    vec->push_back(sParam {8, "Direction Gaussian mean [degree]", false, false, 0, 359, 0, nullptr, 0});
    vec->push_back(sParam {9, "Direction Gaussian std dev [degree]", false, false, 1, 180, 90, nullptr, 0});
    vec->push_back(sParam {10, "Collision avoidance", true, false, 0, 0, 0, new std::vector<QString>{ "None", "Opposite direction" }, 0});
    vec->push_back(sParam {11, "Collision avoidance radius [cm]", false, true, 1, 1000, 200, nullptr, 0});
    return vec;
}

void cGauss_markov_mobility::Set_parameter(int param_id, double value_or_index)
{
    switch ( param_id ) {
    case 0 : seed = value_or_index; break;
    case 1 : time_period_msecs = value_or_index * 1000; break;
    case 2 : alfa = value_or_index; break;
    case 3 : distance_from_aoi_edges_m = value_or_index; break;
    case 4 : speed_mean = value_or_index; break;
    case 5 : speed_normal_mean = value_or_index; speed_normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev)); break;
    case 6 : speed_normal_std_dev = value_or_index; speed_normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev)); break;
    case 7 : dir_mean = value_or_index * pi_per_180; break;
    case 8 : dir_normal_mean = value_or_index * pi_per_180; dir_normal_dist.param(std::normal_distribution<>::param_type(dir_normal_mean, dir_normal_std_dev)); break;
    case 9 : dir_normal_std_dev = value_or_index * pi_per_180; dir_normal_dist.param(std::normal_distribution<>::param_type(dir_normal_mean, dir_normal_std_dev)); break;
    case 10 : collision_avoidance = (eCollision_avoidance)value_or_index;
        if ( collision_avoidance == opposite_dir ) {
            col_avoid_radius_basic = sqrt((*area_m_width * *area_m_height) / ( is_for_sinks_not_nodes ? *number_of_get_items : *number_of_set_items )) * 0.1;
            col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
            widgets.at(2 * 11)->show();
            static_cast<QSlider*>(widgets.at(2 * 11 + 1))->setValue(col_avoid_radius_basic * 100.);
            widgets.at(2 * 11 + 1)->show();
        }else {
            widgets.at(2 * 11)->hide();
            widgets.at(2 * 11 + 1)->hide();
        }
        break;
    case 11 : col_avoid_radius_basic = value_or_index / 100.;
        col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
    }
}

iMobility_interface* cGauss_markov_mobility::Create_new_instance()
{
    return new cGauss_markov_mobility();
}

QString cGauss_markov_mobility::Get_params_string()
{
    QString x("");

    x += "library prefix name,";
    x += "gauss_markov_mobility_plugin";
    x += "\n";

    x += "seed,";
    x += std::to_string(seed);
    x += "\n";

    x += "time period length [s],";
    x += std::to_string((int)((double)time_period_msecs / 1000));
    x += "\n";

    x += "alpha,";
    x += std::to_string(alfa);
    x += "\n";

    x += "distance from AoI edges [m],";
    x += std::to_string(distance_from_aoi_edges_m);
    x += "\n";

    x += "speed mean [km/h],";
    x += std::to_string(speed_mean);
    x += "\n";

    x += "speed Gaussian mean [km/h],";
    x += std::to_string(speed_normal_mean);
    x += "\n";

    x += "speed Gaussian std dev [km/h],";
    x += std::to_string(speed_normal_std_dev);
    x += "\n";

    x += "direction mean [degree],";
    x += std::to_string(dir_mean * (180. / M_PI));
    x += "\n";

    x += "direction Gaussian mean [degree],";
    x += std::to_string(dir_normal_mean * (180. / M_PI));
    x += "\n";

    x += "direction Gaussian std dev [degree],";
    x += std::to_string(dir_normal_std_dev * (180. / M_PI));
    x += "\n";

    x += "collision avoidance,";
    x += (collision_avoidance == none ? "none" : "opposite direction" );

    if ( collision_avoidance == opposite_dir ) {
        x += "\n";
        x += "collision avoidance radius [cm],";
        x += std::to_string(col_avoid_radius_basic * 100.);
    }

    return x;
}
