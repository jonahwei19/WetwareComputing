#ifndef MEA_PATTERN_H
#define MEA_PATTERN_H

#include <array>

constexpr std::size_t mea_x_regions = 12;
constexpr std::size_t mea_y_regions = 10;

struct MeaPattern {
    std::array<std::array<bool, mea_x_regions>, mea_y_regions> regions_to_stimulate;
};

#endif