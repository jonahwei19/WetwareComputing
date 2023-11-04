#include "dlp.hpp"

void Dlp::startRendering() {
    window.create(sf::VideoMode(dlp_resolution_width, dlp_resolution_height), "DLP Output", sf::Style::None);
    window.setPosition(sf::Vector2i(sf::VideoMode::getDesktopMode().width * 0.5 - window.getSize().x * 0.5, 
        sf::VideoMode::getDesktopMode().height * 0.5 - window.getSize().y * 0.5));
    render_thread = std::thread(&Dlp::renderLoop, this);
}

void Dlp::renderLoop() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        const std::lock_guard<std::mutex> lock(mea_pattern->getMutex());

        mea_pattern->generate();
        window.clear(sf::Color::Black);
        window.draw(*mea_pattern);
        window.display();
    }
}