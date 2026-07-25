#include "TestHelpers.hpp"

#include "pvz/core/EntityFactory.hpp"
#include "pvz/core/World.hpp"

#include <gtest/gtest.h>

namespace pvz {
namespace {

using test::advance;
using test::countOf;
using test::findFirst;
using test::makeWorld;
using test::quietConfig;
using test::sendCommand;

World worldWithSunflower(GameConfig config, GridCell cell = {2, 2}) {
    World world = test::makeWorld(std::move(config));
    world.setSun(100);
    sendCommand(world, SelectPlantCommand{PlantType::Sunflower});
    sendCommand(world, ClickCommand{world.geometry().cellCenter(cell)});
    advance(world, 0.05F);
    return world;
}

/// Timers carry their remainder forward. Assigning the interval instead of
/// adding it would discard the leftover fraction each cycle and make
/// production drift slower than configured.
TEST(Economy, SunflowerProducesOnItsInterval) {
    GameConfig config = quietConfig();
    config.sunflower.actionInterval = 10.0F;
    config.sunLifetime = 1000.0F;

    World world = worldWithSunflower(config);

    advance(world, 9.0F);
    EXPECT_EQ(countOf(world, EntityKind::Sun), 0) << "not due yet";

    advance(world, 1.5F);
    EXPECT_EQ(countOf(world, EntityKind::Sun), 1) << "first sun at ~10s";

    advance(world, 10.0F);
    EXPECT_EQ(countOf(world, EntityKind::Sun), 2) << "second sun at ~20s";
}

TEST(Economy, CollectingSunAddsTheConfiguredValue) {
    GameConfig config = quietConfig();
    config.sunValue = 25;
    config.sunLifetime = 1000.0F;
    config.sunDriftSpeed = 0.0F;

    World world = makeWorld(config);
    world.setSun(0);

    const Vec2 where{600.0F, 300.0F};
    world.placeNow(EntityFactory::createSun(where, where, config));

    sendCommand(world, ClickCommand{where});

    EXPECT_EQ(world.sun(), 25);
    advance(world, 0.05F);
    EXPECT_EQ(countOf(world, EntityKind::Sun), 0) << "collected sun is removed";
}

/// A sun drifting over an empty tile must be collectable without the same
/// click also spending that sun on a plant.
TEST(Economy, ClickOnSunDoesNotAlsoPlant) {
    GameConfig config = quietConfig();
    config.sunLifetime = 1000.0F;
    config.sunDriftSpeed = 0.0F;

    World world = makeWorld(config);
    const GridCell cell{2, 3};
    const Vec2 tileCentre = world.geometry().cellCenter(cell);

    world.setSun(100);
    world.placeNow(EntityFactory::createSun(tileCentre, tileCentre, config));

    sendCommand(world, SelectPlantCommand{PlantType::Peashooter});
    sendCommand(world, ClickCommand{tileCentre});
    advance(world, 0.05F);

    EXPECT_FALSE(world.board().isOccupied(cell)) << "the click was consumed by the sun";
    EXPECT_EQ(countOf(world, EntityKind::Peashooter), 0);
    EXPECT_EQ(world.sun(), 100 + config.sunValue);
}

/// Uncollected sun used to pile up on the lawn forever.
TEST(Economy, UncollectedSunExpires) {
    GameConfig config = quietConfig();
    config.sunLifetime = 3.0F;
    config.sunDriftSpeed = 0.0F;

    World world = makeWorld(config);
    const Vec2 where{600.0F, 300.0F};
    world.placeNow(EntityFactory::createSun(where, where, config));

    advance(world, 2.0F);
    EXPECT_EQ(countOf(world, EntityKind::Sun), 1) << "still within its lifetime";

    advance(world, 1.5F);
    EXPECT_EQ(countOf(world, EntityKind::Sun), 0) << "should have expired";
}

TEST(Economy, SunDriftsTowardsItsTargetAndStops) {
    GameConfig config = quietConfig();
    config.sunLifetime = 1000.0F;
    config.sunDriftSpeed = 60.0F;

    World world = makeWorld(config);
    const Vec2 start{500.0F, 300.0F};
    const Vec2 target{560.0F, 300.0F};
    world.placeNow(EntityFactory::createSun(start, target, config));

    const Entity* sun = findFirst(world, EntityKind::Sun);
    ASSERT_NE(sun, nullptr);

    advance(world, 0.5F);
    EXPECT_GT(sun->position().x, start.x);
    EXPECT_LT(sun->position().x, target.x + 0.01F);

    advance(world, 3.0F);
    EXPECT_NEAR(sun->position().x, target.x, 0.01F) << "must settle exactly on target";
    EXPECT_NEAR(sun->position().y, target.y, 0.01F);
}

TEST(Economy, StartingSunMatchesConfig) {
    GameConfig config = quietConfig();
    config.startingSun = 75;
    const World world = makeWorld(config);
    EXPECT_EQ(world.sun(), 75);
}

} // namespace
} // namespace pvz
