#include "pvz/app/ResourceManager.hpp"

#include "pvz/app/AssetPaths.hpp"

#include <utility>

namespace pvz::app {

TextureId textureFor(EntityKind kind) noexcept {
    switch (kind) {
        case EntityKind::Peashooter:
            return TextureId::Peashooter;
        case EntityKind::Sunflower:
            return TextureId::Sunflower;
        case EntityKind::Wallnut:
            return TextureId::Wallnut;
        case EntityKind::BasicZombie:
            return TextureId::BasicZombie;
        case EntityKind::Projectile:
            return TextureId::Projectile;
        case EntityKind::Sun:
            return TextureId::Sun;
    }
    return TextureId::Sun;
}

ResourceManager::ResourceManager(std::filesystem::path assetRoot) : m_root(std::move(assetRoot)) {}

void ResourceManager::loadTexture(TextureId id, const char* filename) {
    const std::filesystem::path path = m_root / filename;
    // loadFromFile returns bool; the original discarded all seven results, so a
    // missing file only surfaced as a crash on the first draw.
    if (!m_textures[static_cast<std::size_t>(id)].loadFromFile(path)) {
        throw AssetLoadError{path, "failed to load texture"};
    }
    m_textures[static_cast<std::size_t>(id)].setSmooth(true);
}

void ResourceManager::loadAll() {
    loadTexture(TextureId::Map, "map.png");
    loadTexture(TextureId::Peashooter, "peashooter.png");
    loadTexture(TextureId::Sunflower, "sunflower.png");
    loadTexture(TextureId::Wallnut, "wallnut.png");
    loadTexture(TextureId::BasicZombie, "viktor.png");
    loadTexture(TextureId::Projectile, "projectile.png");
    loadTexture(TextureId::Sun, "sun.png");

    const std::filesystem::path fontPath = m_root / "main_font.ttf";
    if (!m_font.openFromFile(fontPath)) {
        throw AssetLoadError{fontPath, "failed to load font"};
    }
}

const sf::Texture& ResourceManager::texture(TextureId id) const {
    return m_textures[static_cast<std::size_t>(id)];
}

} // namespace pvz::app
