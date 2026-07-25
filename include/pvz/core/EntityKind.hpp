#pragma once

#include <cstdint>
#include <string_view>

namespace pvz {

/// Concrete entity type.
///
/// A single tag replaces the three incompatible type-identification schemes
/// the original used: dynamic_cast in per-frame loops, string equality, and a
/// `name.find("zombie")` substring search.
enum class EntityKind : std::uint8_t {
    Peashooter,
    Sunflower,
    Wallnut,
    BasicZombie,
    Projectile,
    Sun,
};

inline constexpr std::size_t kEntityKindCount = 6;

/// Broad behavioural grouping, used for the per-step view partitioning.
enum class EntityCategory : std::uint8_t {
    Plant,
    Zombie,
    Projectile,
    Pickup,
};

[[nodiscard]] constexpr EntityCategory categoryOf(EntityKind kind) noexcept {
    switch (kind) {
        case EntityKind::Peashooter:
        case EntityKind::Sunflower:
        case EntityKind::Wallnut:
            return EntityCategory::Plant;
        case EntityKind::BasicZombie:
            return EntityCategory::Zombie;
        case EntityKind::Projectile:
            return EntityCategory::Projectile;
        case EntityKind::Sun:
            return EntityCategory::Pickup;
    }
    return EntityCategory::Pickup;
}

/// Draw order: plants sit under zombies, which sit under projectiles/pickups.
[[nodiscard]] constexpr int renderLayerOf(EntityKind kind) noexcept {
    switch (categoryOf(kind)) {
        case EntityCategory::Plant:
            return 0;
        case EntityCategory::Zombie:
            return 1;
        case EntityCategory::Projectile:
        case EntityCategory::Pickup:
            return 2;
    }
    return 2;
}

/// Returns a view into a string literal; no allocation, unlike the original
/// `Stat<std::string> name` which was copied by value inside a sort comparator.
[[nodiscard]] constexpr std::string_view toString(EntityKind kind) noexcept {
    switch (kind) {
        case EntityKind::Peashooter:
            return "peashooter";
        case EntityKind::Sunflower:
            return "sunflower";
        case EntityKind::Wallnut:
            return "wallnut";
        case EntityKind::BasicZombie:
            return "basiczombie";
        case EntityKind::Projectile:
            return "projectile";
        case EntityKind::Sun:
            return "sun";
    }
    return "unknown";
}

/// The three plants the player can place.
enum class PlantType : std::uint8_t {
    Peashooter,
    Wallnut,
    Sunflower,
};

inline constexpr std::size_t kPlantTypeCount = 3;

[[nodiscard]] constexpr EntityKind kindOf(PlantType type) noexcept {
    switch (type) {
        case PlantType::Peashooter:
            return EntityKind::Peashooter;
        case PlantType::Wallnut:
            return EntityKind::Wallnut;
        case PlantType::Sunflower:
            return EntityKind::Sunflower;
    }
    return EntityKind::Peashooter;
}

[[nodiscard]] constexpr std::string_view toString(PlantType type) noexcept {
    return toString(kindOf(type));
}

} // namespace pvz
