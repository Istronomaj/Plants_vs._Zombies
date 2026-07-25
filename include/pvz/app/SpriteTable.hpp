#pragma once

#include "pvz/app/ResourceManager.hpp"
#include "pvz/core/EntityKind.hpp"

#include <array>

namespace pvz::app {

/// How one entity kind is drawn.
struct SpriteSpec {
    TextureId texture{};
    /// Target on-screen height in map-space units. Scale is derived from this
    /// and the texture's own size, so swapping in a different-resolution
    /// sprite does not change how big it appears.
    float displayHeight{};
};

/// One table indexed by EntityKind, replacing the original's two parallel
/// string-keyed maps (paths and scale factors) which could silently disagree:
/// the map texture had a path but no scale entry, and the scale lookup used
/// .at(), so any mismatch threw during rendering.
[[nodiscard]] constexpr SpriteSpec spriteSpecFor(EntityKind kind) noexcept {
    switch (kind) {
        case EntityKind::Peashooter:
            return {TextureId::Peashooter, 49.0F};
        case EntityKind::Sunflower:
            return {TextureId::Sunflower, 57.0F};
        case EntityKind::Wallnut:
            return {TextureId::Wallnut, 62.0F};
        case EntityKind::BasicZombie:
            return {TextureId::BasicZombie, 113.0F};
        case EntityKind::Projectile:
            return {TextureId::Projectile, 27.0F};
        case EntityKind::Sun:
            return {TextureId::Sun, 44.0F};
    }
    return {TextureId::Sun, 40.0F};
}

} // namespace pvz::app
