#include "random_walk_mobility_plugin.h"
#include "cslider.h"
#include "cfp_slider.h"
#include <QComboBox>
#include <QSlider>

cRandom_walk_mobility::cRandom_walk_mobility()
{
    seed = 0;
    time_period_msecs = 5000;
    is_global_not_local_cs = true;
    dir_range_start = 0.;
    dir_range_end = dir_range_len = two_pi;
    speed_range_start = 1;
    speed_range_end = 50;
    speed_range_len = speed_range_end - speed_range_start;
    is_speed_uniform_not_normal = true;
    speed_normal_mean = 25;
    speed_normal_std_dev = 10;
    collision_avoidance = none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

QString cRandom_walk_mobility::Alg_name()
{
    return "Random Walk";
}

bool cRandom_walk_mobility::Is_for_sinks()
{
    return true;
}

bool cRandom_walk_mobility::Is_for_nodes()
{
    return true;
}

void cRandom_walk_mobility::Get_client_params(std::vector<double> *set_pos_vec, int *number_of_set_items, const std::vector<double> *get_pos_vec,
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

void cRandom_walk_mobility::Get_widgets(std::vector<QWidget*> widgets)
{
    this->widgets = widgets;
    this->widgets.at(2 * 8)->hide();
    this->widgets.at(2 * 8 + 1)->hide();
    this->widgets.at(2 * 9)->hide();
    this->widgets.at(2 * 9 + 1)->hide();
    this->widgets.at(2 * 11)->hide();
    this->widgets.at(2 * 11 + 1)->hide();
}

void cRandom_walk_mobility::Unit_len_changed()
{
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
}

void cRandom_walk_mobility::Resize_happened(double new_old_rate)
{
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
//    for ( int i = 0; i < *number_of_set_items; ++i ) {
//        double_positions.at(i * 2) *= new_old_rate;
//        double_positions.at(i * 2 + 1) *= new_old_rate;
//    }
}

bool cRandom_walk_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = double_positions.at(a * 2) - double_positions.at(b * 2);
    double delta_y = double_positions.at(a * 2 + 1) - double_positions.at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cRandom_walk_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = double_positions.at(a * 2) - get_vec->at(b * 2);
    double delta_y = double_positions.at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cRandom_walk_mobility::Init_run()
{
    if ( is_speed_uniform_not_normal ) {
        Speed_gen_fn = [this](int i){ return ((double) speed_gens[i]() / speed_gens[i].max()) * speed_range_len + speed_range_start; };
    }else {
        normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        Speed_gen_fn = [this](int i){ double a = normal_dist(speed_gens[i]); return a > speed_range_end ? speed_range_end : a < speed_range_start ? speed_range_start : a; };
    }
    cum_time = 0;
    act_dirs.clear();
    act_speeds.clear();
    double_positions.clear();
    is_in_coll_rad.clear();
    dir_gens.clear();
    speed_gens.clear();
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        dir_gens.push_back(std::mt19937(seed + i));
        speed_gens.push_back(std::mt19937(seed + i + 1234567));
        act_dirs.push_back(((double) dir_gens[i]() / dir_gens[i].max()) * dir_range_len + dir_range_start);
        act_speeds.push_back(Speed_gen_fn(i));
        double_positions.push_back(set_pos_vec->at(i * 2));
        double_positions.push_back(set_pos_vec->at(i * 2 + 1));
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

void cRandom_walk_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
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
            if ( tmpx <= *area_m_width ) {
                if ( tmpx >= 0 ) {
                    double_positions.at(i * 2) = tmpx;
                }else {
                    double_positions.at(i * 2) = -tmpx;
                    act_dirs.at(i) = M_PI - act_dirs.at(i);
                    if ( act_dirs.at(i) < 0 ) {
                        act_dirs.at(i) += two_pi;
                    }
                }
            }else {
                double_positions.at(i * 2) = 2 * (*area_m_width) - tmpx;
                act_dirs.at(i) = M_PI - act_dirs.at(i);
                if ( act_dirs.at(i) < 0 ) {
                    act_dirs.at(i) += two_pi;
                }
            }
            double tmpy = double_positions.at(i * 2 + 1) + vt * sin(act_dirs[i]);
            if ( tmpy <= *area_m_height ) {
                if ( tmpy >= 0 ) {
                    double_positions.at(i * 2 + 1) = tmpy;
                }else {
                    double_positions.at(i * 2 + 1) = -tmpy;
                    act_dirs.at(i) = two_pi - act_dirs.at(i);
                }
            }else {
                double_positions.at(i * 2 + 1) = 2 * (*area_m_height) - tmpy;
                act_dirs.at(i) = two_pi - act_dirs.at(i);
            }
            if ( is_global_not_local_cs ) {
                act_dirs.at(i) = ((double) dir_gens[i]() / dir_gens[i].max()) * dir_range_len + dir_range_start;
            }else {
                act_dirs.at(i) += ((double) dir_gens[i]() / dir_gens[i].max()) * dir_range_len - dir_range_len / 2;
                if ( act_dirs.at(i) < 0 ) {
                    act_dirs.at(i) += two_pi;
                } else if ( act_dirs.at(i) > two_pi ) {
                    act_dirs.at(i) -= two_pi;
                }
            }
            act_speeds.at(i) = Speed_gen_fn(i);
        }
        cum_time = diff;
    }

    double size_rate = (double)diff / 3600;
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        double vt = size_rate * act_speeds[i];
        double tmpx = double_positions.at(i * 2) + vt * cos(act_dirs[i]);
        if ( tmpx <= *area_m_width ) {
            if ( tmpx >= 0 ) {
                double_positions.at(i * 2) = tmpx;
            }else {
                double_positions.at(i * 2) = -tmpx;
                act_dirs.at(i) = M_PI - act_dirs.at(i);
                if ( act_dirs.at(i) < 0 ) {
                    act_dirs.at(i) += two_pi;
                }
            }
        }else {
            double_positions.at(i * 2) = 2 * (*area_m_width) - tmpx;
            act_dirs.at(i) = M_PI - act_dirs.at(i);
            if ( act_dirs.at(i) < 0 ) {
                act_dirs.at(i) += two_pi;
            }
        }
        double tmpy = double_positions.at(i * 2 + 1) + vt * sin(act_dirs[i]);
        if ( tmpy <= *area_m_height ) {
            if ( tmpy >= 0 ) {
                double_positions.at(i * 2 + 1) = tmpy;
            }else {
                double_positions.at(i * 2 + 1) = -tmpy;
                act_dirs.at(i) = two_pi - act_dirs.at(i);
            }
        }else {
            double_positions.at(i * 2 + 1) = 2 * (*area_m_height) - tmpy;
            act_dirs.at(i) = two_pi - act_dirs.at(i);
        }
        set_pos_vec->at(i * 2) = double_positions.at(i * 2);
        set_pos_vec->at(i * 2 + 1) = double_positions.at(i * 2 + 1);
    }
}

void cRandom_walk_mobility::Set_parameter(std::string param_str, std::string ertek_str)
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
        }else if ( param_str == "time period t [s]" ) {
            reinterpret_cast<cSlider*>(widgets[3])->setValue(value_or_index);
        }else if ( param_str == "direction range start [degree]" ) {
            reinterpret_cast<cSlider*>(widgets[7])->setValue(value_or_index);
        }else if ( param_str == "direction range end [degree]" ) {
            reinterpret_cast<cSlider*>(widgets[9])->setValue(value_or_index);
        }else if ( param_str == "speed range start [km/h]" ) {
            reinterpret_cast<cSlider*>(widgets[11])->setValue(value_or_index);
        }else if ( param_str == "speed range end [km/h]" ) {
            reinterpret_cast<cSlider*>(widgets[13])->setValue(value_or_index);
        }else if ( param_str == "speed Gaussian mean [km/h]" ) {
            reinterpret_cast<cFp_slider*>(widgets[17])->Set_fp_value(value_or_index);
        }else if (param_str == "speed Gaussian std dev [km/h]"  ) {
            reinterpret_cast<cFp_slider*>(widgets[19])->Set_fp_value(value_or_index);
        }else if ( param_str == "collision avoidance radius [cm]" ) {
            reinterpret_cast<cSlider*>(widgets[23])->setValue(value_or_index);
        }
    }else {
        if ( ertek_str[ertek_str.size() - 1] == '\r' ) {
            ertek_str.resize(ertek_str.size() - 1);
        }
        if ( param_str == "type of coordinate system" ) {
            int tmp = (ertek_str.compare("global") == 0 ? 0 : 1);
            reinterpret_cast<QComboBox*>(widgets[5])->setCurrentIndex(tmp);
        }else if ( param_str == "speed distribution" ) {
            int tmp = (ertek_str.compare("uniform") == 0 ? 0 : 1);
            reinterpret_cast<QComboBox*>(widgets[15])->setCurrentIndex(tmp);
        }else if ( param_str == "collision avoidance" ) {
            int tmp = (ertek_str.compare("opposite direction") == 0 ? 1 : 0);
            reinterpret_cast<QComboBox*>(widgets[21])->setCurrentIndex(tmp);
        }
    }
}

std::vector<cRandom_walk_mobility::sParam>* cRandom_walk_mobility::Get_parameters()
{
    auto vec = new std::vector<cRandom_walk_mobility::sParam>();
    vec->push_back(sParam {0, "Seed", false, true, 0, 1000, 0, nullptr, 0});
    vec->push_back(sParam {1, "Time period t [s]", false, true, 1, 120, 5, nullptr, 0});
    vec->push_back(sParam {2, "Type of coordinate system", true, false, 0, 0, 0, new std::vector<QString>{"Global", "Local"}, 0});
    vec->push_back(sParam {3, "Direction range start [degree]", false, true, 0, 360, 0, nullptr, 0});
    vec->push_back(sParam {4, "Direction range end [degree]", false, true, 0, 360, 360, nullptr, 0});
    vec->push_back(sParam {5, "Speed range start [km/h]", false, true, 0, 200, 1, nullptr, 0});
    vec->push_back(sParam {6, "Speed range end [km/h]", false, true, 1, 200, 50, nullptr, 0});
    vec->push_back(sParam {7, "Speed distribution", true, false, 0, 0, 0, new std::vector<QString>{"Uniform", "Gaussian"}, 0});
    vec->push_back(sParam {8, "Speed Gaussian mean [km/h]", false, false, 1, 200, 25, nullptr, 0});
    vec->push_back(sParam {9, "Speed Gaussian std dev [km/h]", false, false, 1, 100, 10, nullptr, 0});
    vec->push_back(sParam {10, "Collision avoidance", true, false, 0, 0, 0, new std::vector<QString>{ "None", "Opposite direction" }, 0});
    vec->push_back(sParam {11, "Collision avoidance radius [cm]", false, true, 1, 1000, 200, nullptr, 0});
    return vec;
}

void cRandom_walk_mobility::Set_parameter(int param_id, double value_or_index)
{
    switch ( param_id ) {
    case 0 : seed = value_or_index; break;
    case 1 : time_period_msecs = value_or_index * 1000; break;
    case 2 : is_global_not_local_cs = (value_or_index > 0 ? false : true); break;
    case 3 : dir_range_start = value_or_index * pi_per_180; dir_range_len = dir_range_end - dir_range_start; break;
    case 4 : dir_range_end = value_or_index * pi_per_180; dir_range_len = dir_range_end - dir_range_start; break;
    case 5 : speed_range_start = value_or_index; speed_range_len = speed_range_end - speed_range_start; break;
    case 6 : speed_range_end = value_or_index; speed_range_len = speed_range_end - speed_range_start; break;
    case 7 : is_speed_uniform_not_normal = (value_or_index > 0 ? false : true);
        if ( is_speed_uniform_not_normal ) {
            widgets.at(2 * 8)->hide();
            widgets.at(2 * 8 + 1)->hide();
            widgets.at(2 * 9)->hide();
            widgets.at(2 * 9 + 1)->hide();
        }else {
            widgets.at(2 * 8)->show();
            widgets.at(2 * 8 + 1)->show();
            widgets.at(2 * 9)->show();
            widgets.at(2 * 9 + 1)->show();
        }
        break;
    case 8 : speed_normal_mean = value_or_index; normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev)); break;
    case 9 : speed_normal_std_dev = value_or_index; normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev)); break;
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

iMobility_interface* cRandom_walk_mobility::Create_new_instance()
{
    return new cRandom_walk_mobility();
}

QString cRandom_walk_mobility::Get_params_string()
{
    QString x("");

    x += "library prefix name,";
    x += "random_walk_mobility_plugin";
    x += "\n";

    x += "seed,";
    x += std::to_string(seed);
    x += "\n";

    x += "time period t [s],";
    x += std::to_string((int)((double)time_period_msecs / 1000));
    x += "\n";

    x += "type of coordinate system,";
    x += (is_global_not_local_cs ? "global" : "local");
    x += "\n";

    x += "direction range start [degree],";
    x += std::to_string(dir_range_start * (180. / M_PI));
    x += "\n";

    x += "direction range end [degree],";
    x += std::to_string(dir_range_end * (180. / M_PI));
    x += "\n";

    x += "speed range start [km/h],";
    x += std::to_string(speed_range_start);
    x += "\n";

    x += "speed range end [km/h],";
    x += std::to_string(speed_range_end);
    x += "\n";

    x += "speed distribution,";
    x += (is_speed_uniform_not_normal ? "uniform" : "Gaussian");
    x += "\n";

    if ( ! is_speed_uniform_not_normal ) {
        x += "speed Gaussian mean [km/h],";
        x += std::to_string(speed_normal_mean);
        x += "\n";

        x += "speed Gaussian std dev [km/h],";
        x += std::to_string(speed_normal_std_dev);
        x += "\n";
    }

    x += "collision avoidance,";
    x += (collision_avoidance == none ? "none" : "opposite direction" );

    if ( collision_avoidance == opposite_dir ) {
        x += "\n";
        x += "collision avoidance radius [cm],";
        x += std::to_string(col_avoid_radius_basic * 100.);
    }

    return x;
}
