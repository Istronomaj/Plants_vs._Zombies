#pragma once

#include "pvz/app/ResourceManager.hpp"
#include "pvz/core/Entity.hpp"
#include "pvz/core/Vec2.hpp"
#include "pvz/core/World.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <vector>

namespace pvz::app {

/// Converts between the simulation's map space and window pixels.
class ViewTransform {
public:
    explicit ViewTransform(float scale) noexcept : m_scale(scale) {}

    [[nodiscard]] sf::Vector2f toScreen(Vec2 worldPosition) const noexcept {
        return {worldPosition.x * m_scale, worldPosition.y * m_scale};
    }

    [[nodiscard]] Vec2 toWorld(sf::Vector2i screenPosition) const noexcept {
        return {static_cast<float>(screenPosition.x) / m_scale,
                static_cast<float>(screenPosition.y) / m_scale};
    }

    [[nodiscard]] float scale() const noexcept { return m_scale; }

private:
    float m_scale;
};

/// Draws the lawn and everything on it.
class GameRenderer {
public:
    GameRenderer(const ResourceManager& resources, ViewTransform view);

    void draw(sf::RenderTarget& target, const World& world);

private:
    const ResourceManager* m_resources;
    ViewTransform m_view;
    sf::Sprite m_mapSprite;
    sf::Sprite m_entitySprite;
    /// Reused across frames so steady-state rendering does not allocate. The
    /// original rebuilt a vector, sorted it with a comparator that returned
    /// std::string by value, and constructed a fresh sf::Sprite per entity,
    /// every frame.
    std::vector<const Entity*> m_drawList;
};

} // namespace pvz::app
