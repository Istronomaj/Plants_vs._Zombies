#pragma once

#include "pvz/core/EntityKind.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <cstdint>
#include <filesystem>

namespace pvz::app {

/// Texture identity.
///
/// An enum rather than a string key: a typo cannot compile, and there is no
/// way for the texture table and a separate scale table to disagree.
enum class TextureId : std::uint8_t {
    Map,
    Peashooter,
    Sunflower,
    Wallnut,
    BasicZombie,
    Projectile,
    Sun,
};

inline constexpr std::size_t kTextureCount = 7;

[[nodiscard]] TextureId textureFor(EntityKind kind) noexcept;

/// Owns every texture and the font.
///
/// Loading is eager and fails loudly at startup, naming the offending file.
class ResourceManager {
public:
    explicit ResourceManager(std::filesystem::path assetRoot);

    /// Loads everything. Throws AssetLoadError on the first failure.
    void loadAll();

    [[nodiscard]] const sf::Texture& texture(TextureId id) const;
    [[nodiscard]] const sf::Font& font() const noexcept { return m_font; }
    [[nodiscard]] const std::filesystem::path& assetRoot() const noexcept { return m_root; }

private:
    void loadTexture(TextureId id, const char* filename);

    std::filesystem::path m_root;
    std::array<sf::Texture, kTextureCount> m_textures;
    sf::Font m_font;
};

} // namespace pvz::app
