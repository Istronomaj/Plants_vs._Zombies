#include "pvz/app/GameRenderer.hpp"

#include "pvz/app/SpriteTable.hpp"

#include <algorithm>

namespace pvz::app {

GameRenderer::GameRenderer(const ResourceManager& resources, ViewTransform view)
    : m_resources(&resources),
      m_view(view),
      // SFML 3 removed the default constructors for Sprite and Text, so both
      // must be given a resource here rather than assigned later.
      m_mapSprite(resources.texture(TextureId::Map)),
      m_entitySprite(resources.texture(TextureId::Sun)) {
    m_mapSprite.setScale({m_view.scale(), m_view.scale()});
    m_drawList.reserve(256);
}

void GameRenderer::draw(sf::RenderTarget& target, const World& world) {
    target.draw(m_mapSprite);

    m_drawList.clear();
    for (const auto& entity : world.entities()) {
        if (entity->isAlive()) {
            m_drawList.push_back(entity.get());
        }
    }

    // Stable so entities on the same layer keep a consistent relative order
    // between frames instead of flickering.
    std::stable_sort(m_drawList.begin(), m_drawList.end(),
                     [](const Entity* a, const Entity* b) {
                         return a->renderLayer() < b->renderLayer();
                     });

    for (const Entity* entity : m_drawList) {
        const SpriteSpec spec = spriteSpecFor(entity->kind());
        const sf::Texture& texture = m_resources->texture(spec.texture);

        m_entitySprite.setTexture(texture, /*resetRect=*/true);

        const auto textureSize = sf::Vector2f{texture.getSize()};
        if (textureSize.y <= 0.0F) {
            continue;
        }

        // Scale from the desired on-screen height, so a sprite swapped for one
        // of a different resolution still draws at the same size.
        const float scale = (spec.displayHeight * m_view.scale()) / textureSize.y;
        m_entitySprite.setScale({scale, scale});
        m_entitySprite.setOrigin(textureSize / 2.0F);
        m_entitySprite.setPosition(m_view.toScreen(entity->position()));

        target.draw(m_entitySprite);
    }
}

} // namespace pvz::app
