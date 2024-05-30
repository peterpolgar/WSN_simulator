#include "poisson_disk_placement_plugin.h"
#include "poisson_disk_sampling.h"
#include "cslider.h"
#include <cmath>

cPoisson_disk_placement::cPoisson_disk_placement()
{
    seed = 0;
}

QString cPoisson_disk_placement::Alg_name()
{
    return "Poisson";
}

bool cPoisson_disk_placement::Is_for_sinks()
{
    return true;
}

bool cPoisson_disk_placement::Is_for_nodes()
{
    return true;
}

void cPoisson_disk_placement::Compute_initial_position(std::vector<double> *pos_vec, int number_of_items, int width, int height)
{
    pos_vec->clear();
    auto kRadius = float(((double)width / std::sqrt((double)number_of_items / ((double)height / width))) * 0.786);
    constexpr auto kXMin = std::array<float, 2>{{0.F, 0.F}};
    auto kXMax = std::array<float, 2>{{(float)width, (float)height}};
    auto coords = thinks::PoissonDiskSampling(kRadius, kXMin, kXMax, 30, seed);
    int count = 0;
    for (const auto &pair : coords) {
        pos_vec->push_back(pair[0]);
        pos_vec->push_back(pair[1]);
        ++count;
        if ( count == number_of_items ) {
            break;
        }
    }
//    qDebug() << kRadius << " " << width << " " << height << " " << number_of_items;
}

void cPoisson_disk_placement::Set_parameter(std::string param_str, std::string ertek_str)
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
        }
    }
}

std::vector<cPoisson_disk_placement::sParam>* cPoisson_disk_placement::Get_parameters()
{
    auto vec = new std::vector<cPoisson_disk_placement::sParam>();
    sParam seed {0, "Seed", false, true, 0, 1000, 0, nullptr, 0};
    vec->push_back(seed);
    return vec;
}

void cPoisson_disk_placement::Set_parameter(int param_id, double value_or_index)
{
    if ( param_id == 0 ) {
        seed = value_or_index;
    }
}

iPlacement_interface* cPoisson_disk_placement::Create_new_instance()
{
    return new cPoisson_disk_placement();
}

void cPoisson_disk_placement::Get_widgets(std::vector<QWidget*> widgets)
{
    this->widgets = widgets;
}
