#include <stdexcept>
#include "directed_unique_direction_mobility_plugin.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" EXPORT iMobility_interface* getObj(void) {
    return (iMobility_interface*)(new cDirected_unique_direction_mobility);
}

cDirected_unique_direction_mobility::cDirected_unique_direction_mobility()
{
    seed = 0;
    speed = 25;

    collision_avoidance = eCollision_avoidance::none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

std::string cDirected_unique_direction_mobility::Alg_name()
{
    return "Directed unique-direction";
}

void cDirected_unique_direction_mobility::Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
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

bool cDirected_unique_direction_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = set_pos_vec->at(a * 2) - set_pos_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - set_pos_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cDirected_unique_direction_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = set_pos_vec->at(a * 2) - get_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cDirected_unique_direction_mobility::Init_run()
{
    gen.seed(seed);
    the_dir = ((double) gen() / gen.max()) * two_pi;
    the_opposite_dir = the_dir + M_PI;
    if ( the_opposite_dir >= two_pi ) {
        the_opposite_dir -= two_pi;
    }
    act_dirs.clear();
    is_in_coll_rad.clear();
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        act_dirs.push_back(the_dir);
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

void cDirected_unique_direction_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
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
                        act_dirs[i] = ( act_dirs[i] == the_dir ? the_opposite_dir : the_dir );
                    }
                    break;
                }
            }
            if ( tmpb == false ) {
                for ( int j = 0; j < civ; ++j ) {
                    if ( Is_dist_lesser_than_rad_with_get(i, j, tmp_tmp_get_vec) ) {
                        tmpb = true;
                        if ( ! is_in_coll_rad.at(i) ) {
                            act_dirs[i] = ( act_dirs[i] == the_dir ? the_opposite_dir : the_dir );
                        }
                        break;
                    }
                }
            }
            is_in_coll_rad.at(i) = tmpb;
        }
    }

    double size_rate = (double)time_in_msec / 3600.;
    double vt = size_rate * speed;
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        double tmpx = set_pos_vec->at(i * 2) + vt * cos(act_dirs[i]);
        double tmpy = set_pos_vec->at(i * 2 + 1) + vt * sin(act_dirs[i]);
        bool is_dir_changed = false, is_negx_not_overflow = false;
        if ( tmpx <= *area_width ) {
            if ( tmpx < 0 ) {
                is_dir_changed = true;
                is_negx_not_overflow = true;
            }
        }else {
            is_dir_changed = true;
        }
        if ( tmpy <= *area_height ) {
            if ( tmpy >= 0 ) {
                if ( is_dir_changed ) {
                    if ( is_negx_not_overflow ) {
                        set_pos_vec->at(i * 2 + 1) -= (vt + 2 * set_pos_vec->at(i * 2) / cos(act_dirs[i])) * sin(act_dirs[i]);
                        set_pos_vec->at(i * 2) = -tmpx;
                    }else {
                        set_pos_vec->at(i * 2 + 1) -= (vt + 2 * (set_pos_vec->at(i * 2) - *area_width) / cos(act_dirs[i])) * sin(act_dirs[i]);
                        set_pos_vec->at(i * 2) = 2 * (*area_width) - tmpx;
                    }
                    act_dirs[i] = ( act_dirs[i] == the_dir ? the_opposite_dir : the_dir );
                }else {
                    set_pos_vec->at(i * 2) = tmpx;
                    set_pos_vec->at(i * 2 + 1) = tmpy;
                }
            }else {
                set_pos_vec->at(i * 2) -= (vt + 2 * set_pos_vec->at(i * 2 + 1) / sin(act_dirs[i])) * cos(act_dirs[i]);
                set_pos_vec->at(i * 2 + 1) = -tmpy;
                act_dirs[i] = ( act_dirs[i] == the_dir ? the_opposite_dir : the_dir );
            }
        }else {
            set_pos_vec->at(i * 2) -= (vt + 2 * (set_pos_vec->at(i * 2 + 1) - *area_height) / sin(act_dirs[i])) * cos(act_dirs[i]);
            set_pos_vec->at(i * 2 + 1) = 2 * (*area_height) - tmpy;
            act_dirs[i] = ( act_dirs[i] == the_dir ? the_opposite_dir : the_dir );
        }
    }
}

void cDirected_unique_direction_mobility::Set_parameter(std::string param_str, std::string ertek_str)
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
        }else if ( param_str == "speed [km/h]" ) {
            speed = value_or_index;
        }else if ( param_str == "collision avoidance radius [cm]" ) {
            col_avoid_radius_square = value_or_index / 100.;
            col_avoid_radius_basic = col_avoid_radius_square;
            col_avoid_radius_square *= col_avoid_radius_square;
        }
    }else {
        if ( ertek_str[ertek_str.size() - 1] == '\r' ) {
            ertek_str.resize(ertek_str.size() - 1);
        }
        if ( param_str == "collision avoidance" ) {
            collision_avoidance = (ertek_str.compare("opposite direction") == 0 ? eCollision_avoidance::opposite_dir : eCollision_avoidance::none);
            if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
                col_avoid_radius_basic = sqrt((*area_width * *area_height) / ( is_for_sinks_not_nodes ? *number_of_get_items : *number_of_set_items )) * 0.1;
                col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
            }
        }
    }
}

iMobility_interface* cDirected_unique_direction_mobility::Create_new_instance()
{
    return new cDirected_unique_direction_mobility();
}

std::string cDirected_unique_direction_mobility::Get_params_string()
{
    std::string x("");

    x += "library prefix name,";
    x += "directed_unique_direction_mobility_plugin";
    x += "\n";

    x += "seed,";
    x += std::to_string(seed);
    x += "\n";

    x += "speed [km/h],";
    x += std::to_string(speed);
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
