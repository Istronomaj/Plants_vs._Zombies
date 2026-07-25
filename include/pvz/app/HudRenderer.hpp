#pragma once

#include "pvz/app/ResourceManager.hpp"
#include "pvz/core/EntityKind.hpp"
#include "pvz/core/Rect.hpp"
#include "pvz/core/World.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>

namespace pvz::app {

/// Sun counter, health, wave progress and the seed packet bar.
///
/// Layout is derived from the window size rather than the original's absolute
/// pixel positions, which assumed one specific 2000x858 window.
class HudRenderer {
public:
    HudRenderer(const ResourceManager& resources, sf::Vector2u windowSize);

    void draw(sf::RenderTarget& target, const World& world);

    /// Screen-space rectangle of the seed packet for `type`, so clicks on the
    /// bar can select a plant.
    [[nodiscard]] Rect packetBounds(PlantType type) const noexcept;

    [[nodiscard]] static constexpr int packetCount() noexcept {
        return static_cast<int>(kPlantTypeCount);
    }

private:
    void drawPacket(sf::RenderTarget& target, const World& world, PlantType type);

    const ResourceManager* m_resources;
    sf::Vector2u m_windowSize;

    sf::Text m_sunText;
    sf::Text m_healthText;
    sf::Text m_waveText;
    sf::Text m_packetCostText;

    sf::RectangleShape m_packetBackground;
    sf::Sprite m_packetIcon;

    float m_packetSize{};
    float m_packetGap{};
    float m_packetOriginX{};
    float m_packetOriginY{};
};

} // namespace pvz::app
