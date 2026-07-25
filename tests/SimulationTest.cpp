#include "TestHelpers.hpp"

#include "pvz/core/World.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace pvz {
namespace {

using test::advance;
using test::makeWorld;
using test::quietConfig;

GameConfig busyConfig() {
    GameConfig config{};
    config.zombieCountMin = 12;
    config.zombieCountMax = 12;
    config.firstSpawnDelay = 1.0F;
    return config;
}

/// A comparable summary of everything the simulation decides.
struct Snapshot {
    int sun{};
    int health{};
    int pending{};
    int alive{};
    std::vector<std::tuple<int, float, float, int>> entities;

    friend bool operator==(const Snapshot&, const Snapshot&) = default;
};

Snapshot snapshot(const World& world) {
    Snapshot snap;
    snap.sun = world.sun();
    snap.health = world.playerHealth();
    snap.pending = world.zombiesPending();
    snap.alive = world.zombiesAlive();
    for (const auto& entity : world.entities()) {
        snap.entities.emplace_back(static_cast<int>(entity->kind()), entity->position().x,
                                   entity->position().y, entity->health());
    }
    return snap;
}

/// The whole simulation must be reproducible from a seed. This is the broadest
/// regression net in the suite: it fails if any entity's motion, any timer or
/// any RNG consumption changes.
TEST(Simulation, SameSeedProducesIdenticalOutcome) {
    World a = makeWorld(busyConfig(), 42);
    World b = makeWorld(busyConfig(), 42);

    advance(a, 30.0F);
    advance(b, 30.0F);

    EXPECT_EQ(snapshot(a), snapshot(b));
}

TEST(Simulation, DifferentSeedsProduceDifferentOutcomes) {
    World a = makeWorld(busyConfig(), 1);
    World b = makeWorld(busyConfig(), 999);

    advance(a, 30.0F);
    advance(b, 30.0F);

    EXPECT_NE(snapshot(a), snapshot(b));
}

/// The original slept inside the event-polling loop, so with no events there
/// was no frame pacing at all and the real speed came from vsync -- a 144Hz
/// display ran the game 2.4x faster. With a fixed timestep, how the host
/// chops up wall-clock time cannot affect the result.
TEST(Simulation, IsIndependentOfFrameSize) {
    World steady = makeWorld(busyConfig(), 7);
    World jittery = makeWorld(busyConfig(), 7);

    // A steady 120Hz caller.
    const int totalSteps = 3600;  // 30 seconds
    for (int i = 0; i < totalSteps; ++i) {
        steady.step(kFixedTimeStep, {});
    }

    // A caller delivering uneven 30fps-ish frames through an accumulator,
    // exactly as Application::run does.
    float accumulator = 0.0F;
    const float frameTimes[] = {1.0F / 30.0F, 1.0F / 45.0F, 1.0F / 20.0F, 1.0F / 60.0F};
    int consumed = 0;
    int frameIndex = 0;
    while (consumed < totalSteps) {
        accumulator += frameTimes[frameIndex % 4];
        ++frameIndex;
        while (accumulator >= kFixedTimeStep && consumed < totalSteps) {
            jittery.step(kFixedTimeStep, {});
            accumulator -= kFixedTimeStep;
            ++consumed;
        }
    }

    EXPECT_EQ(snapshot(steady), snapshot(jittery));
}

TEST(Spawner, WaitsForTheGracePeriodBeforeTheFirstZombie) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 5;
    config.zombieCountMax = 5;
    config.firstSpawnDelay = 5.0F;
    config.spawnIntervalMin = 3.0F;
    config.spawnIntervalMax = 3.0F;

    World world = makeWorld(config);

    advance(world, 4.0F);
    EXPECT_EQ(world.zombiesAlive(), 0) << "grace period should still be running";

    advance(world, 1.5F);
    EXPECT_EQ(world.zombiesAlive(), 1) << "first zombie should have arrived";
}

TEST(Spawner, SpawnsExactlyTheConfiguredNumber) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 6;
    config.zombieCountMax = 6;
    config.firstSpawnDelay = 0.2F;
    config.spawnIntervalMin = 0.2F;
    config.spawnIntervalMax = 0.2F;
    config.basicZombie.speed = 0.0F;

    World world = makeWorld(config);
    advance(world, 5.0F);

    EXPECT_EQ(world.zombiesPending(), 0);
    EXPECT_EQ(world.zombiesAlive(), 6);
}

TEST(Spawner, PlacesZombiesOnValidLanesAtTheSpawnLine) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 20;
    config.zombieCountMax = 20;
    config.firstSpawnDelay = 0.1F;
    config.spawnIntervalMin = 0.1F;
    config.spawnIntervalMax = 0.1F;
    config.basicZombie.speed = 0.0F;

    World world = makeWorld(config);
    advance(world, 4.0F);

    const GridGeometry geom = world.geometry();
    int seen = 0;
    for (const auto& entity : world.entities()) {
        if (entity->kind() != EntityKind::BasicZombie) {
            continue;
        }
        ++seen;
        EXPECT_FLOAT_EQ(entity->position().x, geom.spawnX());

        bool onALane = false;
        for (int row = 0; row < geom.rows(); ++row) {
            if (entity->position().y == geom.laneY(row)) {
                onALane = true;
                break;
            }
        }
        EXPECT_TRUE(onALane) << "zombie spawned off-lane at y=" << entity->position().y;
    }
    EXPECT_EQ(seen, 20);
}

/// Spawn count is drawn once per match from the configured range.
TEST(Spawner, RespectsTheConfiguredCountRange) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 15;
    config.zombieCountMax = 30;

    for (std::uint32_t seed = 1; seed <= 25; ++seed) {
        const World world = makeWorld(config, seed);
        EXPECT_GE(world.zombiesRemaining(), 15) << "seed " << seed;
        EXPECT_LE(world.zombiesRemaining(), 30) << "seed " << seed;
    }
}

TEST(Simulation, ZombiesAdvanceAtTheConfiguredSpeed) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 60.0F;

    World world = makeWorld(config);
    const GridGeometry geom = world.geometry();
    world.setZombiesPending(1);
    world.placeNow(EntityFactory::createBasicZombie({geom.spawnX(), geom.laneY(0)}, config));

    const Entity* zombie = test::findFirst(world, EntityKind::BasicZombie);
    ASSERT_NE(zombie, nullptr);
    const float startX = zombie->position().x;

    advance(world, 1.0F);

    EXPECT_NEAR(zombie->position().x, startX - 60.0F, 1.0F);
}

} // namespace
} // namespace pvz
