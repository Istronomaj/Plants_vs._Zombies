#include "pvz/app/ScreenRenderer.hpp"

namespace pvz::app {
namespace {

void centreOn(sf::Text& text, float centreX, float y) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition({centreX - bounds.size.x / 2.0F - bounds.position.x, y});
}

/// Draws each line separately so every one is individually centred. Setting a
/// multi-line string on a single sf::Text centres the block, which leaves the
/// shorter lines visibly off-axis.
void drawCentredLines(sf::RenderTarget& target, sf::Text& text, const std::string& lines,
                      float centreX, float firstLineY, float lineSpacing) {
    std::size_t start = 0;
    float y = firstLineY;
    while (start <= lines.size()) {
        const std::size_t end = lines.find('\n', start);
        const std::string line =
            lines.substr(start, end == std::string::npos ? std::string::npos : end - start);

        if (!line.empty()) {
            text.setString(line);
            centreOn(text, centreX, y);
            target.draw(text);
        }

        y += lineSpacing;
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
}

} // namespace

ScreenRenderer::ScreenRenderer(const ResourceManager& resources, sf::Vector2u windowSize)
    : m_windowSize(windowSize),
      m_title(resources.font(), "", 96),
      m_subtitle(resources.font(), "", 40) {
    m_dimmer.setSize(sf::Vector2f{windowSize});

    const auto height = static_cast<float>(windowSize.y);
    m_title.setCharacterSize(static_cast<unsigned int>(height * 0.14F));
    m_title.setOutlineColor(sf::Color::Black);
    m_title.setOutlineThickness(4.0F);

    m_subtitle.setCharacterSize(static_cast<unsigned int>(height * 0.055F));
    m_subtitle.setFillColor(sf::Color{225, 225, 235});
    m_subtitle.setOutlineColor(sf::Color::Black);
    m_subtitle.setOutlineThickness(2.0F);
}

void ScreenRenderer::drawPanel(sf::RenderTarget& target, const std::string& title,
                               const std::string& subtitle, sf::Color titleColor,
                               std::uint8_t dimAlpha) {
    m_dimmer.setFillColor(sf::Color{0, 0, 0, dimAlpha});
    target.draw(m_dimmer);

    const auto centreX = static_cast<float>(m_windowSize.x) / 2.0F;
    const auto height = static_cast<float>(m_windowSize.y);

    m_title.setString(title);
    m_title.setFillColor(titleColor);
    centreOn(m_title, centreX, height * 0.30F);
    target.draw(m_title);

    drawCentredLines(target, m_subtitle, subtitle, centreX, height * 0.55F,
                     static_cast<float>(m_subtitle.getCharacterSize()) * 1.5F);
}

void ScreenRenderer::drawMainMenu(sf::RenderTarget& target) {
    drawPanel(target, "Plants vs. Zombies",
              "Press Enter to play\n1 / 2 / 3 to pick a plant, click to place it\nEsc to pause",
              sf::Color{120, 220, 120}, 180);
}

void ScreenRenderer::drawPauseOverlay(sf::RenderTarget& target) {
    drawPanel(target, "Paused", "Esc to resume  -  R to restart", sf::Color{225, 225, 235}, 140);
}

void ScreenRenderer::drawResult(sf::RenderTarget& target, bool won) {
    drawPanel(target, won ? "You survived!" : "The zombies ate your brains",
              "R to play again  -  Esc to quit",
              won ? sf::Color{120, 220, 120} : sf::Color{225, 90, 90}, 170);
}

} // namespace pvz::app
