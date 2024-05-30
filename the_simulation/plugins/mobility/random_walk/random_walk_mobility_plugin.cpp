#include <stdexcept>
#include <cstring>
#include <iostream>
#include "random_walk_mobility_plugin.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" EXPORT iMobility_interface* getObj(void) {
    return (iMobility_interface*)(new cRandom_walk_mobility);
}

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
    collision_avoidance = eCollision_avoidance::none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

std::string cRandom_walk_mobility::Alg_name()
{
    return "Random Walk";
}

void cRandom_walk_mobility::Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
                                   unsigned *number_of_get_items, double *width, double *height, double *sink_radius, double *node_radius, bool is_for_sinks_not_nodes)
{
    this->set_pos_vec = set_pos_vec;
    this->number_of_set_items = number_of_set_items;
    this->get_pos_vec = get_pos_vec;
    this->number_of_get_items = number_of_get_items;
    this->area_width = width;
    this->area_height = height;
    this->sink_radius = sink_radius;
    this->node_radius = node_radius;
    this->is_for_sinks_not_nodes = is_for_sinks_not_nodes;
}

bool cRandom_walk_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = set_pos_vec->at(a * 2) - set_pos_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - set_pos_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cRandom_walk_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = set_pos_vec->at(a * 2) - get_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cRandom_walk_mobility::Init_run()
{
    if ( is_speed_uniform_not_normal ) {
        Speed_gen_fn = [this](unsigned i){ return ((double) speed_gens[i]() / speed_gens[i].max()) * speed_range_len + speed_range_start; };
    }else {
        normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        Speed_gen_fn = [this](unsigned i){ double a = normal_dist(speed_gens[i]); return a > speed_range_end ? speed_range_end : a < speed_range_start ? speed_range_start : a; };
    }
    cum_time = 0;
    act_dirs.clear();
    act_speeds.clear();
    is_in_coll_rad.clear();
    dir_gens.clear();
    speed_gens.clear();
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        dir_gens.push_back(std::mt19937(seed + i));
        speed_gens.push_back(std::mt19937(seed + i + 1234567));
        act_dirs.push_back(((double) dir_gens[i]() / dir_gens[i].max()) * dir_range_len + dir_range_start);
        act_speeds.push_back(Speed_gen_fn(i));
        is_in_coll_rad.push_back(false);
    }
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        if ( ! is_in_coll_rad.at(i) ) {
            for ( unsigned j = 0; j < *number_of_set_items; ++j ) {
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
    if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
        const std::vector<double> *tmp_tmp_get_vec = (get_tmp_pos_vec == nullptr ? get_pos_vec : get_tmp_pos_vec);
        const unsigned civ = *number_of_get_items;
        for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
            bool tmpb = false;
            for ( unsigned j = 0; j < *number_of_set_items; ++j ) {
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
                for ( unsigned j = 0; j < civ; ++j ) {
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
        for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
            double vt = size_rate * act_speeds[i];
            double tmpx = set_pos_vec->at(i * 2) + vt * cos(act_dirs[i]);
            if ( tmpx <= *area_width ) {
                if ( tmpx >= 0 ) {
                    set_pos_vec->at(i * 2) = tmpx;
                }else {
                    set_pos_vec->at(i * 2) = -tmpx;
                    act_dirs.at(i) = M_PI - act_dirs.at(i);
                    if ( act_dirs.at(i) < 0 ) {
                        act_dirs.at(i) += two_pi;
                    }
                }
            }else {
                set_pos_vec->at(i * 2) = 2 * (*area_width) - tmpx;
                act_dirs.at(i) = M_PI - act_dirs.at(i);
                if ( act_dirs.at(i) < 0 ) {
                    act_dirs.at(i) += two_pi;
                }
            }
            double tmpy = set_pos_vec->at(i * 2 + 1) + vt * sin(act_dirs[i]);
            if ( tmpy <= *area_height ) {
                if ( tmpy >= 0 ) {
                    set_pos_vec->at(i * 2 + 1) = tmpy;
                }else {
                    set_pos_vec->at(i * 2 + 1) = -tmpy;
                    act_dirs.at(i) = two_pi - act_dirs.at(i);
                }
            }else {
                set_pos_vec->at(i * 2 + 1) = 2 * (*area_height) - tmpy;
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
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        double vt = size_rate * act_speeds[i];
        double tmpx = set_pos_vec->at(i * 2) + vt * cos(act_dirs[i]);
        if ( tmpx <= *area_width ) {
            if ( tmpx >= 0 ) {
                set_pos_vec->at(i * 2) = tmpx;
            }else {
                set_pos_vec->at(i * 2) = -tmpx;
                act_dirs.at(i) = M_PI - act_dirs.at(i);
                if ( act_dirs.at(i) < 0 ) {
                    act_dirs.at(i) += two_pi;
                }
            }
        }else {
            set_pos_vec->at(i * 2) = 2 * (*area_width) - tmpx;
            act_dirs.at(i) = M_PI - act_dirs.at(i);
            if ( act_dirs.at(i) < 0 ) {
                act_dirs.at(i) += two_pi;
            }
        }
        double tmpy = set_pos_vec->at(i * 2 + 1) + vt * sin(act_dirs[i]);
        if ( tmpy <= *area_height ) {
            if ( tmpy >= 0 ) {
                set_pos_vec->at(i * 2 + 1) = tmpy;
            }else {
                set_pos_vec->at(i * 2 + 1) = -tmpy;
                act_dirs.at(i) = two_pi - act_dirs.at(i);
            }
        }else {
            set_pos_vec->at(i * 2 + 1) = 2 * (*area_height) - tmpy;
            act_dirs.at(i) = two_pi - act_dirs.at(i);
        }
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
            seed = value_or_index;
        }else if ( param_str == "time period t [s]" ) {
            time_period_msecs = value_or_index * 1000;
        }else if ( param_str == "direction range start [degree]" ) {
            dir_range_start = value_or_index * pi_per_180;
            dir_range_len = dir_range_end - dir_range_start;
        }else if ( param_str == "direction range end [degree]" ) {
            dir_range_end = value_or_index * pi_per_180;
            dir_range_len = dir_range_end - dir_range_start;
        }else if ( param_str == "speed range start [km/h]" ) {
            speed_range_start = value_or_index;
            speed_range_len = speed_range_end - speed_range_start;
        }else if ( param_str == "speed range end [km/h]" ) {
            speed_range_end = value_or_index;
            speed_range_len = speed_range_end - speed_range_start;
        }else if ( param_str == "speed Gaussian mean [km/h]" ) {
            speed_normal_mean = value_or_index;
            normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        }else if (param_str == "speed Gaussian std dev [km/h]"  ) {
            speed_normal_std_dev = value_or_index;
            normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        }else if ( param_str == "collision avoidance radius [cm]" ) {
            col_avoid_radius_square = value_or_index / 100.;
            col_avoid_radius_basic = col_avoid_radius_square;
            col_avoid_radius_square *= col_avoid_radius_square;
        }
    }else {
        if ( ertek_str[ertek_str.size() - 1] == '\r' ) {
            ertek_str.resize(ertek_str.size() - 1);
        }
        if ( param_str == "type of coordinate system" ) {
            is_global_not_local_cs = (ertek_str.compare("global") == 0 ? true : false);
        }else if ( param_str == "speed distribution" ) {
            is_speed_uniform_not_normal = (ertek_str.compare("uniform") == 0 ? true : false);
        }else if ( param_str == "collision avoidance" ) {
            collision_avoidance = (ertek_str.compare("opposite direction") == 0 ? eCollision_avoidance::opposite_dir : eCollision_avoidance::none);
            if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
                col_avoid_radius_basic = sqrt((*area_width * *area_height) / ( is_for_sinks_not_nodes ? *number_of_get_items : *number_of_set_items )) * 0.1;
                col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
            }
        }
    }
}

iMobility_interface* cRandom_walk_mobility::Create_new_instance()
{
    return new cRandom_walk_mobility();
}

std::string cRandom_walk_mobility::Get_params_string()
{
    std::string x("");

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
    x += (collision_avoidance == eCollision_avoidance::none ? "eCollision_avoidance::none" : "opposite direction" );

    if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
        x += "\n";
        x += "collision avoidance radius [cm],";
        x += std::to_string(col_avoid_radius_basic * 100.);
    }

    return x;
}