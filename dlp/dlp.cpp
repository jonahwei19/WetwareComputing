#include "dlp.hpp"

void Dlp::start() {
    window.create(sf::VideoMode(dlp_resolution_width, dlp_resolution_height), "DLP Output", sf::Style::None);
    renderLoop(); // TODO: launch a thread
}

void Dlp::renderLoop() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            mea_pattern->generate();

            window.clear(sf::Color::Black);
            window.draw(*mea_pattern);
            window.display();
        }
    }
}