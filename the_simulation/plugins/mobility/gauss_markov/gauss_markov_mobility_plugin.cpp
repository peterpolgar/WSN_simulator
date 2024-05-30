#include <stdexcept>
#include "gauss_markov_mobility_plugin.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" EXPORT iMobility_interface* getObj(void) {
    return (iMobility_interface*)(new cGauss_markov_mobility);
}

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
    distance_from_aoi_edges = 5.;

    collision_avoidance = eCollision_avoidance::none;
    col_avoid_radius_basic = 2.;
    col_avoid_radius_square = 4.;
}

std::string cGauss_markov_mobility::Alg_name()
{
    return "Gauss-Markov";
}

void cGauss_markov_mobility::Get_client_params(std::vector<double> *set_pos_vec, unsigned *number_of_set_items, const std::vector<double> *get_pos_vec,
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

bool cGauss_markov_mobility::Is_dist_lesser_than_rad(int a, int b)
{
    double delta_x = set_pos_vec->at(a * 2) - set_pos_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - set_pos_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

bool cGauss_markov_mobility::Is_dist_lesser_than_rad_with_get(int a, int b, const std::vector<double> *get_vec)
{
    double delta_x = set_pos_vec->at(a * 2) - get_vec->at(b * 2);
    double delta_y = set_pos_vec->at(a * 2 + 1) - get_vec->at(b * 2 + 1);
    return delta_x * delta_x + delta_y * delta_y < col_avoid_radius_square;
}

void cGauss_markov_mobility::Init_run()
{
    cum_time = 0;
    act_dirs.clear();
    act_speeds.clear();
    act_dir_means.clear();
    is_in_coll_rad.clear();
    dir_gens.clear();
    speed_gens.clear();
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        dir_gens.push_back(std::mt19937(seed + i));
        speed_gens.push_back(std::mt19937(seed + i + 1234567));
        act_dirs.push_back(((double) dir_gens[i]() / dir_gens[i].max()) * two_pi);
        act_speeds.push_back(((double) speed_gens[i]() / speed_gens[i].max()) * 50 + 1);
        if ( (*set_pos_vec)[i * 2] < distance_from_aoi_edges ) {
            if ( (*set_pos_vec)[i * 2 + 1] < distance_from_aoi_edges ) {
                act_dir_means.push_back(pi_per_4);
            }else if ( (*set_pos_vec)[i * 2 + 1] > *area_height - distance_from_aoi_edges ) {
                act_dir_means.push_back(fok_315_in_rad);
            }else {
                act_dir_means.push_back(0.);
            }
        }else if ( (*set_pos_vec)[i * 2] > *area_width - distance_from_aoi_edges ) {
            if ( (*set_pos_vec)[i * 2 + 1] < distance_from_aoi_edges ) {
                act_dir_means.push_back(fok_135_in_rad);
            }else if ( (*set_pos_vec)[i * 2 + 1] > *area_height - distance_from_aoi_edges ) {
                act_dir_means.push_back(fok_225_in_rad);
            }else {
                act_dir_means.push_back(M_PI);
            }
        }else if ( (*set_pos_vec)[i * 2 + 1] < distance_from_aoi_edges ) {
            act_dir_means.push_back(pi_per_2);
        }else if ( (*set_pos_vec)[i * 2 + 1] > *area_height - distance_from_aoi_edges ) {
            act_dir_means.push_back(fok_270_in_rad);
        }else {
            act_dir_means.push_back(dir_mean);
        }
        act_dirs[i] = Dir_gen_fn(i);
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

void cGauss_markov_mobility::Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec)
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

    cum_time += time_in_msec;
    int diff = time_in_msec;
    if ( cum_time > time_period_msecs ) {
        diff = cum_time - time_period_msecs;
        if ( diff > time_in_msec ) { diff = time_in_msec; }
        double size_rate = (double)(time_in_msec - diff) / 3600.;
        for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
            double vt = size_rate * act_speeds[i];
            double tmpx = set_pos_vec->at(i * 2) + vt * cos(act_dirs[i]);
            bool x_not_changed = true;
            if ( tmpx <= *area_width ) {
                if ( tmpx < 0 ) {
                    x_not_changed = false;
                }
            }else {
                x_not_changed = false;
            }
            if ( x_not_changed ) {
                double tmpy = set_pos_vec->at(i * 2 + 1) + vt * sin(act_dirs[i]);
                if ( tmpy <= *area_height ) {
                    if ( tmpy >= 0 ) {
                        set_pos_vec->at(i * 2 + 1) = tmpy;
                        set_pos_vec->at(i * 2) = tmpx;
                    }
                }
            }
            act_speeds.at(i) = Speed_gen_fn(i);
            if ( (*set_pos_vec)[i * 2] < distance_from_aoi_edges ) {
                if ( (*set_pos_vec)[i * 2 + 1] < distance_from_aoi_edges ) {
                    act_dir_means[i] = pi_per_4;
                }else if ( (*set_pos_vec)[i * 2 + 1] > *area_height - distance_from_aoi_edges ) {
                    act_dir_means[i] = fok_315_in_rad;
                }else {
                    act_dir_means[i] = 0.;
                }
            }else if ( (*set_pos_vec)[i * 2] > *area_width - distance_from_aoi_edges ) {
                if ( (*set_pos_vec)[i * 2 + 1] < distance_from_aoi_edges ) {
                    act_dir_means[i] = fok_135_in_rad;
                }else if ( (*set_pos_vec)[i * 2 + 1] > *area_height - distance_from_aoi_edges ) {
                    act_dir_means[i] = fok_225_in_rad;
                }else {
                    act_dir_means[i] = M_PI;
                }
            }else if ( (*set_pos_vec)[i * 2 + 1] < distance_from_aoi_edges ) {
                act_dir_means[i] = pi_per_2;
            }else if ( (*set_pos_vec)[i * 2 + 1] > *area_height - distance_from_aoi_edges ) {
                act_dir_means[i] = fok_270_in_rad;
            }
            act_dirs[i] = Dir_gen_fn(i);
        }
        cum_time = diff;
    }

    double size_rate = (double)diff / 3600.;
    for ( unsigned i = 0; i < *number_of_set_items; ++i ) {
        double vt = size_rate * act_speeds[i];
        double tmpx = set_pos_vec->at(i * 2) + vt * cos(act_dirs[i]);
        bool x_not_changed = true;
        if ( tmpx <= *area_width ) {
            if ( tmpx < 0 ) {
                x_not_changed = false;
            }
        }else {
            x_not_changed = false;
        }
        if ( x_not_changed ) {
            double tmpy = set_pos_vec->at(i * 2 + 1) + vt * sin(act_dirs[i]);
            if ( tmpy <= *area_height ) {
                if ( tmpy >= 0 ) {
                    set_pos_vec->at(i * 2 + 1) = tmpy;
                    set_pos_vec->at(i * 2) = tmpx;
                }
            }
        }
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
            seed = value_or_index;
        }else if ( param_str == "time period length [s]" ) {
            time_period_msecs = value_or_index * 1000;
        }else if ( param_str == "alpha" ) {
            alfa = value_or_index;
        }else if ( param_str == "distance from AoI edges [m]" ) {
            distance_from_aoi_edges = value_or_index; 
        }else if ( param_str == "speed mean [km/h]" ) {
            speed_mean = value_or_index;
        }else if ( param_str == "speed Gaussian mean [km/h]" ) {
            speed_normal_mean = value_or_index;
            speed_normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        }else if ( param_str == "speed Gaussian std dev [km/h]" ) {
            speed_normal_std_dev = value_or_index; speed_normal_dist.param(std::normal_distribution<>::param_type(speed_normal_mean, speed_normal_std_dev));
        }else if ( param_str == "direction mean [degree]" ) {
            dir_mean = value_or_index * pi_per_180;
        }else if ( param_str == "direction Gaussian mean [degree]" ) {
            dir_normal_mean = value_or_index * pi_per_180;
            dir_normal_dist.param(std::normal_distribution<>::param_type(dir_normal_mean, dir_normal_std_dev));
        }else if ( param_str == "direction Gaussian std dev [degree]" ) {
            dir_normal_std_dev = value_or_index * pi_per_180; dir_normal_dist.param(std::normal_distribution<>::param_type(dir_normal_mean, dir_normal_std_dev));
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

iMobility_interface* cGauss_markov_mobility::Create_new_instance()
{
    return new cGauss_markov_mobility();
}

std::string cGauss_markov_mobility::Get_params_string()
{
    std::string x("");

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
    x += std::to_string(distance_from_aoi_edges);
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
    x += (collision_avoidance == eCollision_avoidance::none ? "none" : "opposite direction" );

    if ( collision_avoidance == eCollision_avoidance::opposite_dir ) {
        x += "\n";
        x += "collision avoidance radius [cm],";
        x += std::to_string(col_avoid_radius_basic * 100.);
    }

    return x;
}
