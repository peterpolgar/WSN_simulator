#ifndef PARAMS_ENUM_H
#define PARAMS_ENUM_H

#include <array>
#include <cmath>
#include <vector>

#include "poisson_disk_sampling.h"

void Compute_initial_position(std::vector<double> *pos_vec, int number_of_items, int width, int height, int seed)
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

#endif // PARAMS_ENUM_H