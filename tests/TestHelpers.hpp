#pragma once

#include "pvz/core/Commands.hpp"
#include "pvz/core/EntityFactory.hpp"
#include "pvz/core/GameConfig.hpp"
#include "pvz/core/World.hpp"

#include <cstdint>
#include <memory>

namespace pvz::test {

/// A world whose wave never actually arrives, so tests can exercise one
/// mechanic at a time.
///
/// The wave is kept non-empty on purpose: a match with nothing left to spawn
/// and nothing alive is immediately Won, and a finished World stops stepping.
/// Pushing the spawn delay past any test's horizon keeps the match running
/// without a zombie ever appearing. Tests that want zombies override these.
inline GameConfig quietConfig() {
    GameConfig config{};
    config.zombieCountMin = 1;
    config.zombieCountMax = 1;
    config.firstSpawnDelay = 1.0e6F;
    return config;
}

inline World makeWorld(GameConfig config = quietConfig(), std::uint32_t seed = 1234) {
    return World{std::move(config), std::make_unique<Mt19937Rng>(seed)};
}

/// Advances the simulation in exact fixed steps, as the real loop does.
inline void advance(World& world, float seconds, std::span<const Command> commands = {}) {
    const auto steps = static_cast<int>(seconds / kFixedTimeStep);
    for (int i = 0; i < steps; ++i) {
        world.step(kFixedTimeStep, i == 0 ? commands : std::span<const Command>{});
    }
}

/// Runs a single fixed step carrying the given commands.
inline void sendCommand(World& world, const Command& command) {
    const Command commands[] = {command};
    world.step(kFixedTimeStep, commands);
}

[[nodiscard]] inline int countOf(const World& world, EntityKind kind) {
    int count = 0;
    for (const auto& entity : world.entities()) {
        if (entity->kind() == kind && entity->isAlive()) {
            ++count;
        }
    }
    return count;
}

[[nodiscard]] inline Entity* findFirst(World& world, EntityKind kind) {
    for (const auto& entity : world.entities()) {
        if (entity->kind() == kind && entity->isAlive()) {
            return entity.get();
        }
    }
    return nullptr;
}

} // namespace pvz::test
