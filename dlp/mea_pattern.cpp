#include "mea_pattern.hpp"

void MeaPattern::setRegionStimulation(size_t x, size_t y, bool stimulate) {
    regions_to_stimulate[y][x] = stimulate;
}

void MeaPattern::setGlobalStimulation(bool stimulate) {
    std::for_each(regions_to_stimulate.begin(), regions_to_stimulate.end(), [stimulate](auto& row) { row.fill(stimulate); });
}

void MeaPattern::clearRegionStimulation() {
    setGlobalStimulation(false);
}

void MeaPattern::generate() {
    for (size_t row = 0; row < mea_rows; ++row) {
        for (size_t col = 0; col < mea_cols; ++col) {
            size_t triangle_idx = (row * mea_cols + col) * 6;

            verticies[triangle_idx]   = sf::Vector2f(col * region_width, row * region_height);
            verticies[triangle_idx+1] = sf::Vector2f((col+1) * region_width, row * region_height);
            verticies[triangle_idx+2] = sf::Vector2f(col* region_width, (row+1) * region_height);
            verticies[triangle_idx+3] = sf::Vector2f(col* region_width, (row+1) * region_height);
            verticies[triangle_idx+4] = sf::Vector2f((col+1) * region_width, row * region_height);
            verticies[triangle_idx+5] = sf::Vector2f((col+1) * region_width, (row+1) * region_height);
            
            sf::Color color = regions_to_stimulate[row][col] ? stimulation_color : sf::Color::Black;

            for (size_t i = 0; i < 6; ++i) {
                verticies[triangle_idx+i].color = color;
            }
        }
    }
}

void MeaPattern::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(verticies, states);
}