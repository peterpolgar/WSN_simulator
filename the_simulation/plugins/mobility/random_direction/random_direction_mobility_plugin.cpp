#include <stdexcept>
#include "random_direction_mobility_plugin.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" EXPORT iMobility_interface* getObj(void) {
    return (iMobility_interface*)(new cRandom_direction_mobility);
}

cRandom_direction_mobility::cRandom_direction_mobility()
{
    seed = 0;
    pause_time_msecs = 2000;
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

std::string cRandom_direction_mobility::Alg_name()
{
    return "Random Direction";
}

void cRandom_direction_mobility::Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
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

bool cRandom_direction_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = set_pos_vec->at(a * 2) - set_pos_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - set_pos_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cRandom_direction_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = set_pos_vec->at(a * 2) - get_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cRandom_direction_mobility::Init_run()
{
    if ( is_speed_uniform_not_normal ) {
        Speed_gen_fn = [this](int i){ return ((double) speed_gens[i]() / speed_gens[i].max()) * speed_range_len + speed_range_start; };
    }else {
        normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        Speed_gen_fn = [this](int i){ double a = normal_dist(speed_gens[i]); return a > speed_range_end ? speed_range_end : a < speed_range_start ? speed_range_start : a; };
    }
    act_dirs.clear();
    act_speeds.clear();
    act_pause_times_msec.clear();
    is_in_coll_rad.clear();
    dir_gens.clear();
    speed_gens.clear();
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        dir_gens.push_back(std::mt19937(seed + i));
        speed_gens.push_back(std::mt19937(seed + i + 1234567));
        act_dirs.push_back(((double) dir_gens[i]() / dir_gens[i].max()) * two_pi);
        act_speeds.push_back(Speed_gen_fn(i));
        act_pause_times_msec.push_back(pause_time_msecs);
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

void cRandom_direction_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
{
    if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
        const std::vector<double> *tmp_tmp_get_vec = (get_tmp_pos_vec == nullptr ? get_pos_vec : get_tmp_pos_vec);
        const int civ = *number_of_get_items;
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

    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        bool not_move = true;
        if ( act_pause_times_msec.at(i) >= pause_time_msecs ) {
            if ( set_pos_vec->at(i * 2) >= *area_width ) {
                set_pos_vec->at(i * 2) = *area_width;
                act_pause_times_msec.at(i) = time_in_msec;
            }else if ( set_pos_vec->at(i * 2) <= 0 ) {
                set_pos_vec->at(i * 2) = 0;
                act_pause_times_msec.at(i) = time_in_msec;
            }else if ( set_pos_vec->at(i * 2 + 1) >= *area_height ) {
                set_pos_vec->at(i * 2 + 1) = *area_height;
                act_pause_times_msec.at(i) = time_in_msec;
            }else if ( set_pos_vec->at(i * 2 + 1) <= 0 ) {
                set_pos_vec->at(i * 2 + 1) = 0;
                act_pause_times_msec.at(i) = time_in_msec;
            }else {
                double size_rate = (double)time_in_msec / 3600.;
                double vt = size_rate * act_speeds[i];
                set_pos_vec->at(i * 2) += vt * cos(act_dirs[i]);
                set_pos_vec->at(i * 2 + 1) += vt * sin(act_dirs[i]);
                not_move = false;
            }
        }else {
            act_pause_times_msec.at(i) += time_in_msec;
        }
        if ( act_pause_times_msec.at(i) >= pause_time_msecs && not_move ) {
            double diff_msec = act_pause_times_msec.at(i) - pause_time_msecs;
            double size_rate = diff_msec / 3600.;
            act_speeds.at(i) = Speed_gen_fn(i);
            if ( set_pos_vec->at(i * 2) >= *area_width ) {
                act_dirs[i] = ((double) dir_gens[i]() / dir_gens[i].max()) * M_PI + pi_per_2;
            }else if ( set_pos_vec->at(i * 2) <= 0 ) {
                act_dirs[i] = ((double) dir_gens[i]() / dir_gens[i].max()) * M_PI - pi_per_2;
            }else if ( set_pos_vec->at(i * 2 + 1) >= *area_height ) {
                act_dirs[i] = ((double) dir_gens[i]() / dir_gens[i].max()) * M_PI + M_PI;
            }else {
                act_dirs[i] = ((double) dir_gens[i]() / dir_gens[i].max()) * M_PI;
            }
            double vt = size_rate * act_speeds[i];
            set_pos_vec->at(i * 2) += vt * cos(act_dirs[i]);
            set_pos_vec->at(i * 2 + 1) += vt * sin(act_dirs[i]);
        }
    }
}

void cRandom_direction_mobility::Set_parameter(std::string param_str, std::string ertek_str)
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
            pause_time_msecs = value_or_index * 1000;
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
        if ( param_str == "speed distribution" ) {
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

iMobility_interface* cRandom_direction_mobility::Create_new_instance()
{
    return new cRandom_direction_mobility();
}

std::string cRandom_direction_mobility::Get_params_string()
{
    std::string x("");

    x += "library prefix name,";
    x += "random_direction_mobility_plugin";
    x += "\n";

    x += "seed,";
    x += std::to_string(seed);
    x += "\n";

    x += "time period t [s],";
    x += std::to_string((int)((double)pause_time_msecs / 1000));
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
    x += (collision_avoidance == eCollision_avoidance::none ? "none" : "opposite direction" );

    if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
        x += "\n";
        x += "collision avoidance radius [cm],";
        x += std::to_string(col_avoid_radius_basic * 100.);
    }

    return x;
}
