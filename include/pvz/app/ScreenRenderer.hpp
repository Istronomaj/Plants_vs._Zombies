#pragma once

#include "pvz/app/ResourceManager.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <string>

namespace pvz::app {

/// Title screen, pause overlay and the end-of-match result.
///
/// The original announced the result with std::cout and then closed the window
/// in the same breath, so a player never saw either outcome.
class ScreenRenderer {
public:
    ScreenRenderer(const ResourceManager& resources, sf::Vector2u windowSize);

    void drawMainMenu(sf::RenderTarget& target);
    void drawPauseOverlay(sf::RenderTarget& target);
    void drawResult(sf::RenderTarget& target, bool won);

private:
    void drawPanel(sf::RenderTarget& target, const std::string& title, const std::string& subtitle,
                   sf::Color titleColor, std::uint8_t dimAlpha);

    sf::Vector2u m_windowSize;
    sf::RectangleShape m_dimmer;
    sf::Text m_title;
    sf::Text m_subtitle;
};

} // namespace pvz::app
