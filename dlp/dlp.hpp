#ifndef DLP_H
#define DLP_H

#include <memory>
#include <SFML/Graphics.hpp>
#include "mea_pattern.hpp"

constexpr unsigned int dlp_resolution_width = 800;
constexpr unsigned int dlp_resolution_height = 600;

class Dlp {
public:
    Dlp(std::shared_ptr<MeaPattern> mea_pattern) : mea_pattern(mea_pattern) {}
    void start();

private:
    std::shared_ptr<MeaPattern> mea_pattern;
    sf::RenderWindow window;

    void renderLoop();
};

#endif