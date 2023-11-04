#ifndef MEA_PATTERN_H
#define MEA_PATTERN_H

#include <array>
#include <SFML/Graphics.hpp>

constexpr std::size_t mea_cols = 12;
constexpr std::size_t mea_rows = 10;
static const sf::Color stimulation_color = sf::Color::Blue;

class MeaPattern : public sf::Drawable {
public:
    MeaPattern(unsigned int width, unsigned int height) : region_width(width / mea_cols), region_height(height / mea_rows) {
        for (auto& row : regions_to_stimulate) {
            row.fill(false);
        }
    }

    void setRegionStimulation(size_t x, size_t y, bool stimulate);
    void generate();

private:
    std::array<std::array<bool, mea_cols>, mea_rows> regions_to_stimulate;
    unsigned int region_width;
    unsigned int region_height;
    sf::VertexArray verticies = sf::VertexArray(sf::Triangles, mea_cols * mea_rows * 6); // 2 triangles * 3 verticies per region
    
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

#endif