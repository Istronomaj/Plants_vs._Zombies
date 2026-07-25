#pragma once

#include "pvz/core/Board.hpp"
#include "pvz/core/Commands.hpp"
#include "pvz/core/Entity.hpp"
#include "pvz/core/GameConfig.hpp"
#include "pvz/core/GameState.hpp"
#include "pvz/core/GridGeometry.hpp"
#include "pvz/core/Rng.hpp"

#include <memory>
#include <span>
#include <vector>

namespace pvz {

/// The simulation.
///
/// Replaces the original Grid, which was simultaneously the entity container,
/// the wave spawner, the economy, the player's health, the tile map, the mouse
/// handler and the keyboard handler.
class World {
public:
    World(GameConfig config, std::unique_ptr<Rng> rng);

    /// Advances the simulation by `dt` seconds and applies queued input.
    ///
    /// `dt` is expected to be kFixedTimeStep; the caller runs an accumulator so
    /// the simulation is identical regardless of the display's refresh rate.
    void step(float dt, std::span<const Command> commands);

    /// Discards all progress and starts a fresh match.
    void reset();

    // --- Observation ---
    [[nodiscard]] GameState state() const noexcept { return m_state; }
    [[nodiscard]] int sun() const noexcept { return m_sun; }
    [[nodiscard]] int playerHealth() const noexcept { return m_playerHealth; }
    [[nodiscard]] PlantType selectedPlant() const noexcept { return m_selectedPlant; }
    [[nodiscard]] float elapsedTime() const noexcept { return m_elapsed; }

    /// Zombies still to spawn plus those currently on the lawn.
    [[nodiscard]] int zombiesRemaining() const noexcept { return m_zombiesPending + m_zombiesAlive; }
    [[nodiscard]] int zombiesPending() const noexcept { return m_zombiesPending; }
    [[nodiscard]] int zombiesAlive() const noexcept { return m_zombiesAlive; }

    [[nodiscard]] std::span<const std::unique_ptr<Entity>> entities() const noexcept {
        return m_entities;
    }
    [[nodiscard]] const Board& board() const noexcept { return m_board; }
    [[nodiscard]] const GameConfig& config() const noexcept { return m_config; }
    [[nodiscard]] const GridGeometry& geometry() const noexcept { return m_geometry; }

    // --- Called by entities during their update ---
    void spawn(std::unique_ptr<Entity> entity);
    void damagePlayer(int amount) noexcept;
    void addSun(int amount) noexcept;
    [[nodiscard]] Rng& rng() noexcept { return *m_rng; }

    /// Places an entity immediately rather than at the end of the step, and
    /// registers it with the wave counters if it is a zombie. Intended for
    /// setting up a specific board position, mainly in tests.
    void placeNow(std::unique_ptr<Entity> entity);

    /// Overrides how many zombies are still due to spawn.
    void setZombiesPending(int count) noexcept;

    /// Grants sun outright, bypassing the economy.
    void setSun(int amount) noexcept { m_sun = amount; }

    /// Zombies currently in the given lane. Rebuilt once per step, so the
    /// per-entity scans do not need dynamic_cast over every other entity.
    [[nodiscard]] std::span<Entity* const> zombiesInLane(int row) const noexcept;
    [[nodiscard]] std::span<Entity* const> plants() const noexcept { return m_plantsView; }

    /// Lane index for a world Y coordinate, clamped to the board.
    [[nodiscard]] int laneOf(float y) const noexcept;

private:
    void applyCommand(const Command& command);
    void handleClick(Vec2 worldPosition);
    bool tryCollectSun(Vec2 worldPosition);
    bool tryPlant(GridCell cell, PlantType type);
    void updateSpawner(float dt);
    void spawnZombie();
    void rebuildViews();
    void resolveDeaths();
    void checkVictory() noexcept;
    void startMatch();

    GameConfig m_config;
    GridGeometry m_geometry;
    std::unique_ptr<Rng> m_rng;
    Board m_board;

    std::vector<std::unique_ptr<Entity>> m_entities;
    std::vector<std::unique_ptr<Entity>> m_pendingSpawns;

    /// Non-owning views into m_entities, refreshed at the start of each step.
    std::vector<Entity*> m_plantsView;
    std::vector<std::vector<Entity*>> m_zombiesByLane;

    GameState m_state{GameState::Playing};
    int m_sun{0};
    int m_playerHealth{0};
    PlantType m_selectedPlant{PlantType::Peashooter};

    /// Two counters, not one. The original decremented a single count when a
    /// zombie *spawned* and declared victory when it hit zero, so the player
    /// won the moment the last zombie appeared rather than when it died.
    int m_zombiesPending{0};
    int m_zombiesAlive{0};

    float m_spawnTimer{0.0F};
    float m_elapsed{0.0F};
};

} // namespace pvz
