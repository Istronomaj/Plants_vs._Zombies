#include "TestHelpers.hpp"

#include "pvz/core/EntityFactory.hpp"
#include "pvz/core/GameState.hpp"
#include "pvz/core/World.hpp"

#include <gtest/gtest.h>

namespace pvz {
namespace {

using test::advance;
using test::makeWorld;
using test::quietConfig;

/// The original decremented a single "zombies remaining" counter when a zombie
/// SPAWNED and declared victory once it reached zero, so the player won the
/// instant the final zombie walked on -- with a lawn full of zombies.
TEST(Victory, NotWonWhileSpawnedZombiesAreStillAlive) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 3;
    config.zombieCountMax = 3;
    config.firstSpawnDelay = 0.5F;
    config.spawnIntervalMin = 0.5F;
    config.spawnIntervalMax = 0.5F;
    // Slow enough that no zombie reaches the house during the test.
    config.basicZombie.speed = 1.0F;

    World world = makeWorld(config);
    advance(world, 3.0F);

    ASSERT_EQ(world.zombiesPending(), 0) << "all zombies should have spawned by now";
    ASSERT_GT(world.zombiesAlive(), 0) << "and they should still be alive";
    EXPECT_EQ(world.state(), GameState::Playing)
        << "victory must wait for the zombies to actually die";
}

TEST(Victory, WonOnlyWhenNothingPendingAndNothingAlive) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;

    World world = makeWorld(config);
    world.setZombiesPending(1);

    const GridGeometry geom = world.geometry();
    world.placeNow(EntityFactory::createBasicZombie({geom.spawnX(), geom.laneY(0)}, config));

    advance(world, 0.1F);
    EXPECT_EQ(world.state(), GameState::Playing) << "one pending, one alive";

    world.setZombiesPending(0);
    advance(world, 0.1F);
    EXPECT_EQ(world.state(), GameState::Playing) << "still one alive";

    Entity* zombie = test::findFirst(world, EntityKind::BasicZombie);
    ASSERT_NE(zombie, nullptr);
    zombie->takeDamage(config.basicZombie.health);

    advance(world, 0.1F);
    EXPECT_EQ(world.state(), GameState::Won);
}

/// If the last zombie dies on the same step the player's health runs out, the
/// result must be unambiguous. Loss wins.
TEST(Victory, LossTakesPriorityOverWin) {
    GameConfig config = quietConfig();
    config.playerHealth = 10;
    config.basicZombie.damage = 999;
    config.basicZombie.speed = 1000.0F;

    World world = makeWorld(config);
    world.setZombiesPending(0);

    const GridGeometry geom = world.geometry();
    // Close enough to the house that one step carries it across the line.
    world.placeNow(
        EntityFactory::createBasicZombie({geom.houseX() + 1.0F, geom.laneY(2)}, config));

    advance(world, 0.5F);

    EXPECT_EQ(world.playerHealth(), 0);
    EXPECT_EQ(world.state(), GameState::Lost);
}

TEST(Victory, PlayerLosesWhenHealthReachesZero) {
    GameConfig config = quietConfig();
    config.playerHealth = 20;
    config.basicZombie.damage = 20;
    config.basicZombie.speed = 500.0F;

    World world = makeWorld(config);
    world.setZombiesPending(5);

    const GridGeometry geom = world.geometry();
    world.placeNow(
        EntityFactory::createBasicZombie({geom.houseX() + 5.0F, geom.laneY(1)}, config));

    advance(world, 1.0F);

    EXPECT_EQ(world.playerHealth(), 0);
    EXPECT_EQ(world.state(), GameState::Lost);
}

TEST(Victory, WonImmediatelyWhenThereIsNoWave) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 0;
    config.zombieCountMax = 0;

    World world = makeWorld(config);
    advance(world, 0.1F);

    EXPECT_EQ(world.state(), GameState::Won);
}

TEST(Victory, TerminalStateStopsTheSimulation) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 0;
    config.zombieCountMax = 0;

    World world = makeWorld(config);
    advance(world, 0.1F);
    ASSERT_EQ(world.state(), GameState::Won);

    const float elapsedAtWin = world.elapsedTime();
    advance(world, 2.0F);
    EXPECT_FLOAT_EQ(world.elapsedTime(), elapsedAtWin) << "time must not advance after the match";
}

TEST(Victory, PauseHaltsTimeAndResumeContinuesIt) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 5;
    config.zombieCountMax = 5;

    World world = makeWorld(config);
    advance(world, 0.5F);
    const float before = world.elapsedTime();

    test::sendCommand(world, PauseCommand{});
    EXPECT_EQ(world.state(), GameState::Paused);

    advance(world, 1.0F);
    EXPECT_FLOAT_EQ(world.elapsedTime(), before) << "paused time must not advance";

    test::sendCommand(world, ResumeCommand{});
    EXPECT_EQ(world.state(), GameState::Playing);

    advance(world, 0.5F);
    EXPECT_GT(world.elapsedTime(), before);
}

TEST(Victory, RestartRestoresStartingConditions) {
    GameConfig config = quietConfig();
    config.zombieCountMin = 4;
    config.zombieCountMax = 4;

    World world = makeWorld(config);
    world.setSun(999);
    advance(world, 1.0F);

    test::sendCommand(world, RestartCommand{});

    EXPECT_EQ(world.state(), GameState::Playing);
    EXPECT_EQ(world.sun(), config.startingSun);
    EXPECT_EQ(world.playerHealth(), config.playerHealth);
    EXPECT_EQ(world.zombiesAlive(), 0);
    // Commands are applied at the top of a step and the rest of that step still
    // runs, so the clock reads one tick rather than exactly zero.
    EXPECT_LE(world.elapsedTime(), kFixedTimeStep);
}

TEST(Victory, RestartRevivesAMatchLostEarlier) {
    GameConfig config = quietConfig();
    config.playerHealth = 10;
    config.basicZombie.damage = 999;
    config.basicZombie.speed = 1000.0F;

    World world = makeWorld(config);
    const GridGeometry geom = world.geometry();
    world.placeNow(
        EntityFactory::createBasicZombie({geom.houseX() + 1.0F, geom.laneY(0)}, config));

    advance(world, 0.5F);
    ASSERT_EQ(world.state(), GameState::Lost);

    test::sendCommand(world, RestartCommand{});

    EXPECT_EQ(world.state(), GameState::Playing) << "restart must work from a terminal state";
    EXPECT_EQ(world.playerHealth(), config.playerHealth);
    EXPECT_EQ(test::countOf(world, EntityKind::BasicZombie), 0) << "the lawn must be cleared";
}

} // namespace
} // namespace pvz
