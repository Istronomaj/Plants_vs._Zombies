#pragma once

#include "pvz/core/EntityKind.hpp"
#include "pvz/core/GridGeometry.hpp"
#include "pvz/core/Vec2.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace pvz {

struct PlantStats {
    int cost{};
    int health{};
    /// Seconds between actions. 0 means the plant has no periodic action.
    float actionInterval{};
    int damage{};
    float projectileSpeed{};
};

struct ZombieStats {
    int health{};
    float speed{};
    int damage{};
    float biteInterval{};
};

/// All tunable gameplay values.
///
/// A default-constructed GameConfig is fully playable, so the game runs with
/// no config file present. `loadFromToml` layers an optional override on top.
///
/// Distances are in MAP space; the app layer scales them for the window.
/// Times are in seconds (the original stored integer "ticks" that silently
/// assumed the game ran at exactly 60fps).
struct GameConfig {
    // --- Economy ---
    int startingSun = 20;
    int sunValue = 5;
    int playerHealth = 100;
    /// Uncollected sun despawns after this long.
    float sunLifetime = 12.0F;
    float sunDriftSpeed = 60.0F;
    float sunScatterRadius = 150.0F;

    // --- Waves ---
    int zombieCountMin = 15;
    int zombieCountMax = 30;
    float spawnIntervalMin = 3.0F;
    float spawnIntervalMax = 6.0F;
    /// Grace period before the first zombie, so the player can plant a sunflower.
    float firstSpawnDelay = 5.0F;

    // --- Geometry (map space) ---
    Vec2 gridOrigin{208.0F, 92.0F};
    Vec2 gridCellSize{57.5F, 69.75F};
    int rows = 5;
    int cols = 9;
    float houseX = 180.0F;
    float spawnX = 740.0F;
    Vec2 defaultHitboxSize{30.0F, 30.0F};
    Vec2 projectileHitboxSize{15.0F, 15.0F};
    Vec2 sunHitboxSize{30.0F, 30.0F};
    /// Horizontal offset from a peashooter's centre to where its pea appears.
    float projectileSpawnOffset = 20.0F;

    // --- Entity stats ---
    PlantStats peashooter{
        .cost = 4, .health = 100, .actionInterval = 2.0F, .damage = 30, .projectileSpeed = 120.0F};
    PlantStats wallnut{
        .cost = 5, .health = 400, .actionInterval = 0.0F, .damage = 0, .projectileSpeed = 0.0F};
    PlantStats sunflower{
        .cost = 5, .health = 100, .actionInterval = 10.0F, .damage = 0, .projectileSpeed = 0.0F};
    ZombieStats basicZombie{.health = 150, .speed = 60.0F, .damage = 20, .biteInterval = 2.0F};

    [[nodiscard]] const PlantStats& plant(PlantType type) const noexcept;

    [[nodiscard]] GridGeometry geometry() const noexcept {
        return GridGeometry{gridOrigin, gridCellSize, rows, cols, houseX, spawnX};
    }

    /// Loads an override file. Returns nullopt and fills `error` on failure;
    /// callers are expected to fall back to defaults rather than abort, since
    /// a missing balance file must never stop the game from launching.
    [[nodiscard]] static std::optional<GameConfig> loadFromToml(const std::filesystem::path& path,
                                                               std::string& error);
};

/// Simulation tick length. Fixed so the simulation is deterministic and
/// framerate-independent; 120Hz keeps the fastest entity under 1px per step,
/// which lets discrete AABB collision work without swept tests.
inline constexpr float kFixedTimeStep = 1.0F / 120.0F;

/// Upper bound on a single frame's contribution, to stop a stalled frame from
/// triggering an unbounded catch-up loop.
inline constexpr float kMaxFrameTime = 0.25F;

} // namespace pvz
