#include "directed_multi_direction_mobility_plugin.h"
#include "cslider.h"
#include <QComboBox>
#include <QSlider>

cDirected_multi_direction_mobility::cDirected_multi_direction_mobility()
{
    seed = 0;
    speed = 25;

    collision_avoidance = none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

QString cDirected_multi_direction_mobility::Alg_name()
{
    return "Directed multi-direction";
}

bool cDirected_multi_direction_mobility::Is_for_sinks()
{
    return true;
}

bool cDirected_multi_direction_mobility::Is_for_nodes()
{
    return false;
}

void cDirected_multi_direction_mobility::Get_client_params(std::vector<double> *set_pos_vec, int *number_of_set_items, const std::vector<double> *get_pos_vec,
                                              int *number_of_get_items, int *sink_radius, int *node_radius, int *meter_width, int *meter_height, bool is_for_sinks_not_nodes)
{
    this->set_pos_vec = set_pos_vec;
    this->number_of_set_items = number_of_set_items;
    this->get_pos_vec = get_pos_vec;
    this->number_of_get_items = number_of_get_items;
    this->area_m_width = meter_width;
    this->area_m_height = meter_height;
    this->sink_radius = sink_radius;
    this->node_radius = node_radius;
    this->is_for_sinks_not_nodes = is_for_sinks_not_nodes;
}

void cDirected_multi_direction_mobility::Get_widgets(std::vector<QWidget*> widgets)
{
    this->widgets = widgets;
    this->widgets.at(2 * 3)->hide();
    this->widgets.at(2 * 3 + 1)->hide();
}

void cDirected_multi_direction_mobility::Unit_len_changed()
{
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
}

void cDirected_multi_direction_mobility::Resize_happened(double new_old_rate)
{
//    col_avoid_radius_square = (col_avoid_meter_radius / *area_m_width) * *area_m_width;
//    col_avoid_radius_basic = col_avoid_radius_square;
//    col_avoid_radius_square *= col_avoid_radius_square;
//    for ( int i = 0; i < *number_of_set_items; ++i ) {
//        double_positions.at(i * 2) *= new_old_rate;
//        double_positions.at(i * 2 + 1) *= new_old_rate;
//    }
}

bool cDirected_multi_direction_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = double_positions.at(a * 2) - double_positions.at(b * 2);
    double delta_y = double_positions.at(a * 2 + 1) - double_positions.at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cDirected_multi_direction_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = double_positions.at(a * 2) - get_vec->at(b * 2);
    double delta_y = double_positions.at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cDirected_multi_direction_mobility::Init_run()
{
    act_dirs.clear();
    the_dirs.clear();
    the_opposite_dirs.clear();
    double_positions.clear();
    is_in_coll_rad.clear();
    gens.clear();
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        gens.push_back(std::mt19937(seed + i));
        the_dirs.push_back(((double) gens[i]() / gens[i].max()) * two_pi);
        act_dirs.push_back(the_dirs[i]);
        the_opposite_dirs.push_back(the_dirs[i] + M_PI);
        if ( the_opposite_dirs[i] >= two_pi ) {
            the_opposite_dirs[i] -= two_pi;
        }
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

void cDirected_multi_direction_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
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
                        act_dirs[i] = ( act_dirs[i] == the_dirs[i] ? the_opposite_dirs[i] : the_dirs[i] );
                    }
                    break;
                }
            }
            if ( tmpb == false ) {
                for ( int j = 0; j < civ; ++j ) {
                    if ( Is_dist_lesser_than_rad_with_get(i, j, tmp_tmp_get_vec) ) {
                        tmpb = true;
                        if ( ! is_in_coll_rad.at(i) ) {
                            act_dirs[i] = ( act_dirs[i] == the_dirs[i] ? the_opposite_dirs[i] : the_dirs[i] );
                        }
                        break;
                    }
                }
            }
            is_in_coll_rad.at(i) = tmpb;
        }
    }

    double size_rate = (double)time_in_msec / 3600;
    double vt = size_rate * speed;
    for ( int i = 0; i < *number_of_set_items; ++i ) {
        double tmpx = double_positions.at(i * 2) + vt * cos(act_dirs[i]);
        double tmpy = double_positions.at(i * 2 + 1) + vt * sin(act_dirs[i]);
        bool is_dir_changed = false, is_negx_not_overflow = false;
        if ( tmpx <= *area_m_width ) {
            if ( tmpx < 0 ) {
                is_dir_changed = true;
                is_negx_not_overflow = true;
            }
        }else {
            is_dir_changed = true;
        }
        if ( tmpy <= *area_m_height ) {
            if ( tmpy >= 0 ) {
                if ( is_dir_changed ) {
                    if ( is_negx_not_overflow ) {
                        double_positions.at(i * 2 + 1) -= (vt + 2 * double_positions.at(i * 2) / cos(act_dirs[i])) * sin(act_dirs[i]);
                        double_positions.at(i * 2) = -tmpx;
                    }else {
                        double_positions.at(i * 2 + 1) -= (vt + 2 * (double_positions.at(i * 2) - *area_m_width) / cos(act_dirs[i])) * sin(act_dirs[i]);
                        double_positions.at(i * 2) = 2 * (*area_m_width) - tmpx;
                    }
                    act_dirs[i] = ( act_dirs[i] == the_dirs[i] ? the_opposite_dirs[i] : the_dirs[i] );
                }else {
                    double_positions.at(i * 2) = tmpx;
                    double_positions.at(i * 2 + 1) = tmpy;
                }
            }else {
                double_positions.at(i * 2) -= (vt + 2 * double_positions.at(i * 2 + 1) / sin(act_dirs[i])) * cos(act_dirs[i]);
                double_positions.at(i * 2 + 1) = -tmpy;
                act_dirs[i] = ( act_dirs[i] == the_dirs[i] ? the_opposite_dirs[i] : the_dirs[i] );
            }
        }else {
            double_positions.at(i * 2) -= (vt + 2 * (double_positions.at(i * 2 + 1) - *area_m_height) / sin(act_dirs[i])) * cos(act_dirs[i]);
            double_positions.at(i * 2 + 1) = 2 * (*area_m_height) - tmpy;
            act_dirs[i] = ( act_dirs[i] == the_dirs[i] ? the_opposite_dirs[i] : the_dirs[i] );
        }
        set_pos_vec->at(i * 2) = double_positions.at(i * 2);
        set_pos_vec->at(i * 2 + 1) = double_positions.at(i * 2 + 1);
    }
}

void cDirected_multi_direction_mobility::Set_parameter(std::string param_str, std::string ertek_str)
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
        }else if ( param_str == "speed [km/h]" ) {
            reinterpret_cast<cSlider*>(widgets[3])->setValue(value_or_index);
        }else if ( param_str == "collision avoidance radius [cm]" ) {
            reinterpret_cast<cSlider*>(widgets[7])->setValue(value_or_index);
        }
    }else {
        if ( ertek_str[ertek_str.size() - 1] == '\r' ) {
            ertek_str.resize(ertek_str.size() - 1);
        }
        if ( param_str == "collision avoidance" ) {
            int tmp = (ertek_str.compare("opposite direction") == 0 ? 1 : 0);
            reinterpret_cast<QComboBox*>(widgets[5])->setCurrentIndex(tmp);
        }
    }
}

std::vector<cDirected_multi_direction_mobility::sParam>* cDirected_multi_direction_mobility::Get_parameters()
{
    auto vec = new std::vector<cDirected_multi_direction_mobility::sParam>();
    vec->push_back(sParam {0, "Seed", false, true, 0, 1000, 0, nullptr, 0});
    vec->push_back(sParam {1, "Speed [km/h]", false, true, 1, 200, 25, nullptr, 0});
    vec->push_back(sParam {2, "Collision avoidance", true, false, 0, 0, 0, new std::vector<QString>{ "None", "Opposite direction" }, 0});
    vec->push_back(sParam {3, "Collision avoidance radius [cm]", false, true, 1, 1000, 200, nullptr, 0});
    return vec;
}

void cDirected_multi_direction_mobility::Set_parameter(int param_id, double value_or_index)
{
    switch ( param_id ) {
    case 0 : seed = value_or_index; break;
    case 1 : speed = value_or_index; break;
    case 2 : collision_avoidance = (eCollision_avoidance)value_or_index;
        if ( collision_avoidance == opposite_dir ) {
            col_avoid_radius_basic = sqrt((*area_m_width * *area_m_height) / *number_of_get_items) * 0.1;
            col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
            widgets.at(2 * 3)->show();
            static_cast<QSlider*>(widgets.at(2 * 3 + 1))->setValue(col_avoid_radius_basic * 100.);
            widgets.at(2 * 3 + 1)->show();
        }else {
            widgets.at(2 * 3)->hide();
            widgets.at(2 * 3 + 1)->hide();
        }
        break;
    case 3 : col_avoid_radius_basic = value_or_index / 100.;
        col_avoid_radius_square = col_avoid_radius_basic * col_avoid_radius_basic;
    }
}

iMobility_interface* cDirected_multi_direction_mobility::Create_new_instance()
{
    return new cDirected_multi_direction_mobility();
}

QString cDirected_multi_direction_mobility::Get_params_string()
{
    QString x("");

    x += "library prefix name,";
    x += "directed_multi_direction_mobility_plugin";
    x += "\n";

    x += "seed,";
    x += std::to_string(seed);
    x += "\n";

    x += "speed [km/h],";
    x += std::to_string(speed);
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
