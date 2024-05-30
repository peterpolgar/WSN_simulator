#include <stdexcept>
#include "random_waypoint_mobility_plugin.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" EXPORT iMobility_interface* getObj(void) {
    return (iMobility_interface*)(new cRandom_waypoint_mobility);
}

cRandom_waypoint_mobility::cRandom_waypoint_mobility()
{
    seed = 0;
    speed_range_start = 1;
    speed_range_end = 50;
    speed_range_len = speed_range_end - speed_range_start;
    is_speed_uniform_not_normal = true;
    speed_normal_mean = 25;
    speed_normal_std_dev = 10;
    is_pause_time_constant = true;
    const_pause_time_msec = 2000;
    dyn_pause_min_msec = 0;
    dyn_pause_max_msec = 5000;
    dyn_pause_range_len_msec = dyn_pause_max_msec - dyn_pause_min_msec;

    collision_avoidance = eCollision_avoidance::none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

std::string cRandom_waypoint_mobility::Alg_name()
{
    return "Random Waypoint";
}

void cRandom_waypoint_mobility::Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
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

bool cRandom_waypoint_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = set_pos_vec->at(a * 2) - set_pos_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - set_pos_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cRandom_waypoint_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = set_pos_vec->at(a * 2) - get_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cRandom_waypoint_mobility::Init_run()
{
    if ( is_speed_uniform_not_normal ) {
        Speed_gen_fn = [this](unsigned i){ return ((double) speed_gens[i]() / speed_gens[i].max()) * speed_range_len + speed_range_start; };
    }else {
        normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        Speed_gen_fn = [this](unsigned i){ double a = normal_dist(speed_gens[i]); return a > speed_range_end ? speed_range_end : a < speed_range_start ? speed_range_start : a; };
    }
    act_dirs.clear();
    act_speeds.clear();
    is_in_coll_rad.clear();
    act_pause_times_msec.clear();
    dyn_pause_times_msec.clear();
    destinations_coords.clear();
    iranyultsagok.clear();
    x_gens.clear();
    y_gens.clear();
    speed_gens.clear();
    pause_gens.clear();
    atlo_gens.clear();
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        x_gens.push_back(std::mt19937(seed + i));
        y_gens.push_back(std::mt19937(seed + i + 3456789));
        speed_gens.push_back(std::mt19937(seed + i + 1234567));
        pause_gens.push_back(std::mt19937(seed + i + 2345678));
        atlo_gens.push_back(std::mt19937(seed + i + 4567891));
        act_dirs.push_back(0.);
        act_speeds.push_back(0);
        is_in_coll_rad.push_back(false);
        act_pause_times_msec.push_back(0.);
        destinations_coords.push_back(0.);
        destinations_coords.push_back(0.);
        iranyultsagok.push_back(xpyp);
        dyn_pause_times_msec.push_back( is_pause_time_constant ? 1. : ((double) pause_gens[i]() / pause_gens[i].max()) * dyn_pause_range_len_msec + dyn_pause_min_msec );
        if ( (is_pause_time_constant && const_pause_time_msec == 0) || dyn_pause_times_msec.at(i) == 0. ) {
            act_speeds.at(i) = Speed_gen_fn(i);
            destinations_coords.at(2 * i) = ((double) x_gens[i]() / x_gens[i].max()) * *area_width;
            destinations_coords.at(2 * i + 1) = ((double) y_gens[i]() / y_gens[i].max()) * *area_height;
            double diff_x = destinations_coords.at(2 * i) - set_pos_vec->at(2 * i);
            double diff_y = destinations_coords.at(2 * i + 1) - set_pos_vec->at(2 * i + 1);
            double szog = ( diff_x == 0. ? M_PI / 2. : atan(fabs(diff_y / diff_x)) );
            if ( diff_x >= 0 ) {
                if ( diff_y >= 0 ) {
                    act_dirs.at(i) = szog;
                    iranyultsagok.at(i) = xpyp;
                }else {
                    act_dirs.at(i) = two_pi - szog;
                    iranyultsagok.at(i) = xpyn;
                }
            }else {
                if ( diff_y >= 0 ) {
                    act_dirs.at(i) = M_PI - szog;
                    iranyultsagok.at(i) = xnyp;
                }else {
                    act_dirs.at(i) = M_PI + szog;
                    iranyultsagok.at(i) = xnyn;
                }
            }
        }
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

void cRandom_waypoint_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
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
            if ( tmpb ) {
                act_speeds.at(i) = Speed_gen_fn(i);
                double atlo_fele = sqrt(*area_width * *area_width + *area_height * *area_height) / 2;
                atlo_fele *= (double) atlo_gens[i]() / atlo_gens[i].max();
                double tmp_vec_x = atlo_fele * cos(act_dirs[i]);
                double tmp_vec_y = atlo_fele * sin(act_dirs[i]);
                double tmp_dest_x = set_pos_vec->at(2 * i) + tmp_vec_x;
                double tmp_dest_y = set_pos_vec->at(2 * i + 1) + tmp_vec_y;
                if ( tmp_dest_x > *area_width ) {
                    tmp_dest_x = *area_width;
                    tmp_dest_y = set_pos_vec->at(2 * i + 1) + tmp_vec_y * ((*area_width - set_pos_vec->at(2 * i)) / tmp_vec_x);
                }else if ( tmp_dest_x < 0 ){
                    tmp_dest_x = 0;
                    tmp_dest_y = set_pos_vec->at(2 * i + 1) + tmp_vec_y * (-set_pos_vec->at(2 * i) / tmp_vec_x);
                }
                if ( tmp_dest_y > *area_height ) {
                    tmp_dest_y = *area_height;
                    tmp_dest_x = set_pos_vec->at(2 * i) + tmp_vec_x * ((*area_height - set_pos_vec->at(2 * i + 1)) / tmp_vec_y);
                }else if ( tmp_dest_y < 0 ){
                    tmp_dest_y = 0;
                    tmp_dest_x = set_pos_vec->at(2 * i) + tmp_vec_x * (-set_pos_vec->at(2 * i + 1) / tmp_vec_y);
                }
                destinations_coords.at(2 * i) = tmp_dest_x;
                destinations_coords.at(2 * i + 1) = tmp_dest_y;
                double diff_x = destinations_coords.at(2 * i) - set_pos_vec->at(2 * i);
                double diff_y = destinations_coords.at(2 * i + 1) - set_pos_vec->at(2 * i + 1);
                if ( diff_x >= 0 ) {
                    if ( diff_y >= 0 ) {
                        iranyultsagok.at(i) = xpyp;
                    }else {
                        iranyultsagok.at(i) = xpyn;
                    }
                }else {
                    if ( diff_y >= 0 ) {
                        iranyultsagok.at(i) = xnyp;
                    }else {
                        iranyultsagok.at(i) = xnyn;
                    }
                }
            }
        }
    }

    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        double tmp_pt = ( is_pause_time_constant ? const_pause_time_msec : dyn_pause_times_msec.at(i) );
        bool not_move = true;
        if ( act_pause_times_msec.at(i) >= tmp_pt ) {
            double diff_x = destinations_coords.at(2 * i) - set_pos_vec->at(2 * i);
            double diff_y = destinations_coords.at(2 * i + 1) - set_pos_vec->at(2 * i + 1);
            if ( (diff_x < 0 && (iranyultsagok.at(i) == xpyn || iranyultsagok.at(i) == xpyp)) ||
                 (diff_x > 0 && (iranyultsagok.at(i) == xnyn || iranyultsagok.at(i) == xnyp)) ||
                 (diff_y < 0 && (iranyultsagok.at(i) == xpyp || iranyultsagok.at(i) == xnyp)) ||
                 (diff_y > 0 && (iranyultsagok.at(i) == xnyn || iranyultsagok.at(i) == xpyn)) ) {
                act_pause_times_msec.at(i) = time_in_msec;
                if ( ! is_pause_time_constant ) {
                    dyn_pause_times_msec.at(i) = ((double) pause_gens[i]() / pause_gens[i].max()) * dyn_pause_range_len_msec + dyn_pause_min_msec;
                    tmp_pt = dyn_pause_times_msec.at(i);
                }
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
        if ( act_pause_times_msec.at(i) >= tmp_pt && not_move ) {
            double diff_msec = act_pause_times_msec.at(i) - tmp_pt;
            double size_rate = diff_msec / 3600.;
            act_speeds.at(i) = Speed_gen_fn(i);
            destinations_coords.at(2 * i) = ((double) x_gens[i]() / x_gens[i].max()) * *area_width;
            destinations_coords.at(2 * i + 1) = ((double) y_gens[i]() / y_gens[i].max()) * *area_height;
            double diff_x = destinations_coords.at(2 * i) - set_pos_vec->at(2 * i);
            double diff_y = destinations_coords.at(2 * i + 1) - set_pos_vec->at(2 * i + 1);
            double szog = ( diff_x == 0. ? M_PI / 2. : atan(fabs(diff_y / diff_x)) );
            if ( diff_x >= 0 ) {
                if ( diff_y >= 0 ) {
                    act_dirs.at(i) = szog;
                    iranyultsagok.at(i) = xpyp;
                }else {
                    act_dirs.at(i) = two_pi - szog;
                    iranyultsagok.at(i) = xpyn;
                }
            }else {
                if ( diff_y >= 0 ) {
                    act_dirs.at(i) = M_PI - szog;
                    iranyultsagok.at(i) = xnyp;
                }else {
                    act_dirs.at(i) = M_PI + szog;
                    iranyultsagok.at(i) = xnyn;
                }
            }
            double vt = size_rate * act_speeds[i];
            set_pos_vec->at(i * 2) += vt * cos(act_dirs[i]);
            set_pos_vec->at(i * 2 + 1) += vt * sin(act_dirs[i]);
        }
    }
}

void cRandom_waypoint_mobility::Set_parameter(std::string param_str, std::string ertek_str)
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
        }else if ( param_str == "speed range start [km/h]" ) {
            speed_range_start = value_or_index;
            speed_range_len = speed_range_end - speed_range_start;
        }else if ( param_str == "speed range end [km/h]" ) {
            speed_range_end = value_or_index;
            speed_range_len = speed_range_end - speed_range_start;
        }else if ( param_str == "const pause time [s]" ) {
            const_pause_time_msec = value_or_index * 1000;
        }else if ( param_str == "random pause time range start [s]" ) {
            dyn_pause_min_msec = value_or_index * 1000;
            dyn_pause_range_len_msec = dyn_pause_max_msec - dyn_pause_min_msec;
        }else if ( param_str == "random pause time range end [s]" ) {
            dyn_pause_max_msec = value_or_index * 1000; 
            dyn_pause_range_len_msec = dyn_pause_max_msec - dyn_pause_min_msec;
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
        }else if ( param_str == "pause time type" ) {
            is_pause_time_constant = (ertek_str.compare("constant") == 0 ? true : false);
        }else if ( param_str == "collision avoidance" ) {
            collision_avoidance = (ertek_str.compare("opposite direction") == 0 ? eCollision_avoidance::opposite_dir : eCollision_avoidance::none);
            if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
                col_avoid_radius_basic = sqrt((*area_width * *area_height) / ( is_for_sinks_not_nodes ? *number_of_get_items : *number_of_set_items )) * 0.1;
                col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
            }
        }
    }
}

iMobility_interface* cRandom_waypoint_mobility::Create_new_instance()
{
    return new cRandom_waypoint_mobility();
}

std::string cRandom_waypoint_mobility::Get_params_string()
{
    std::string x("");

    x += "library prefix name,";
    x += "random_waypoint_mobility_plugin";
    x += "\n";

    x += "seed,";
    x += std::to_string(seed);
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

    x += "pause time type,";
    x += (is_pause_time_constant ? "constant" : "random");
    x += "\n";

    if ( is_pause_time_constant ) {
        x += "const pause time [s],";
        x += std::to_string(const_pause_time_msec / 1000);
    }else {
        x += "random pause time range start [s],";
        x += std::to_string(dyn_pause_min_msec / 1000);
        x += "\n";
        x += "random pause time range end [s],";
        x += std::to_string(dyn_pause_max_msec / 1000);
    }
    x += "\n";

    x += "collision avoidance,";
    x += (collision_avoidance == eCollision_avoidance::none ? "none" : "opposite direction" );

    if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
        x += "\n";
        x += "collision avoidance radius [cm],";
        x += std::to_string(col_avoid_radius_basic * 100.);
    }

    return x;
}
