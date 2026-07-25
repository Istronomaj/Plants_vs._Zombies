#include "TestHelpers.hpp"

#include "pvz/core/EntityFactory.hpp"
#include "pvz/core/World.hpp"
#include "pvz/core/Zombie.hpp"

#include <gtest/gtest.h>

namespace pvz {
namespace {

using test::advance;
using test::countOf;
using test::findFirst;
using test::makeWorld;
using test::quietConfig;
using test::sendCommand;

/// The original iterated every entity, damaged each overlapping zombie, and
/// never broke out of the loop, so a single pea hit an entire cluster.
TEST(Combat, ProjectileDamagesOneZombieThenDespawns) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;
    config.basicZombie.health = 500;

    World world = makeWorld(config);
    const GridGeometry geom = world.geometry();
    const float laneY = geom.laneY(2);

    // Two zombies stacked on the same spot, both overlapping the pea.
    world.placeNow(EntityFactory::createBasicZombie({400.0F, laneY}, config));
    world.placeNow(EntityFactory::createBasicZombie({402.0F, laneY}, config));

    world.placeNow(EntityFactory::createProjectile({395.0F, laneY}, 30, 60.0F, config));

    advance(world, 0.5F);

    int damagedCount = 0;
    for (const auto& entity : world.entities()) {
        if (entity->kind() == EntityKind::BasicZombie &&
            entity->health() < config.basicZombie.health) {
            ++damagedCount;
        }
    }

    EXPECT_EQ(damagedCount, 1) << "one pea must not damage a whole cluster";
    EXPECT_EQ(countOf(world, EntityKind::Projectile), 0) << "the pea is consumed on impact";
}

TEST(Combat, ProjectileAppliesExactlyItsDamage) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;

    World world = makeWorld(config);
    const float laneY = world.geometry().laneY(1);

    world.placeNow(EntityFactory::createBasicZombie({400.0F, laneY}, config));
    world.placeNow(EntityFactory::createProjectile({396.0F, laneY}, 30, 60.0F, config));

    advance(world, 0.5F);

    const Entity* zombie = findFirst(world, EntityKind::BasicZombie);
    ASSERT_NE(zombie, nullptr);
    EXPECT_EQ(zombie->health(), config.basicZombie.health - 30);
}

TEST(Combat, ProjectileIgnoresZombiesInOtherLanes) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;

    World world = makeWorld(config);
    const GridGeometry geom = world.geometry();

    world.placeNow(EntityFactory::createBasicZombie({400.0F, geom.laneY(4)}, config));
    world.placeNow(EntityFactory::createProjectile({396.0F, geom.laneY(0)}, 30, 60.0F, config));

    advance(world, 0.3F);

    const Entity* zombie = findFirst(world, EntityKind::BasicZombie);
    ASSERT_NE(zombie, nullptr);
    EXPECT_EQ(zombie->health(), config.basicZombie.health);
}

/// Bites land on a fixed interval. The original compared a tick counter with
/// `== 0`, which silently stops firing if the counter ever steps past zero.
TEST(Combat, ZombieBitesAtTheConfiguredRate) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;
    config.basicZombie.damage = 20;
    config.basicZombie.biteInterval = 2.0F;
    config.wallnut.health = 400;

    World world = makeWorld(config);
    const GridCell cell{2, 4};
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Wallnut});
    sendCommand(world, ClickCommand{world.geometry().cellCenter(cell)});
    advance(world, 0.05F);

    const Entity* plant = findFirst(world, EntityKind::Wallnut);
    ASSERT_NE(plant, nullptr);

    world.placeNow(EntityFactory::createBasicZombie(plant->position(), config));

    // 6.1s at one bite per 2s -> exactly three bites.
    advance(world, 6.1F);

    EXPECT_EQ(plant->health(), 400 - 3 * 20);
}

TEST(Combat, ZombieStopsMovingWhileEating) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 60.0F;
    config.wallnut.health = 10000;

    World world = makeWorld(config);
    const GridCell cell{1, 4};
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Wallnut});
    sendCommand(world, ClickCommand{world.geometry().cellCenter(cell)});
    advance(world, 0.05F);

    const Entity* plant = findFirst(world, EntityKind::Wallnut);
    ASSERT_NE(plant, nullptr);

    world.placeNow(EntityFactory::createBasicZombie(plant->position(), config));
    const Entity* zombie = findFirst(world, EntityKind::BasicZombie);
    ASSERT_NE(zombie, nullptr);

    advance(world, 0.05F);
    const float xAfterContact = zombie->position().x;

    advance(world, 3.0F);

    EXPECT_FLOAT_EQ(zombie->position().x, xAfterContact) << "an eating zombie must not advance";
}

TEST(Combat, ZombieResumesWalkingAfterEatingThePlant) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 60.0F;
    config.basicZombie.damage = 100;
    config.basicZombie.biteInterval = 0.5F;
    config.wallnut.health = 100;

    World world = makeWorld(config);
    const GridCell cell{1, 4};
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Wallnut});
    sendCommand(world, ClickCommand{world.geometry().cellCenter(cell)});
    advance(world, 0.05F);

    const Entity* plant = findFirst(world, EntityKind::Wallnut);
    ASSERT_NE(plant, nullptr);
    world.placeNow(EntityFactory::createBasicZombie(plant->position(), config));
    const Entity* zombie = findFirst(world, EntityKind::BasicZombie);
    ASSERT_NE(zombie, nullptr);

    advance(world, 1.0F);
    ASSERT_EQ(countOf(world, EntityKind::Wallnut), 0) << "plant should be eaten by now";

    const float xAfterEating = zombie->position().x;
    advance(world, 1.0F);

    EXPECT_LT(zombie->position().x, xAfterEating) << "zombie must resume walking";
}

/// The original looped over five hard-coded house coordinates and could apply
/// damage more than once for a single zombie.
TEST(Combat, ZombieReachingHouseDamagesPlayerOnceThenDespawns) {
    GameConfig config = quietConfig();
    config.playerHealth = 100;
    config.basicZombie.damage = 20;
    config.basicZombie.speed = 60.0F;

    World world = makeWorld(config);
    world.setZombiesPending(1);

    const GridGeometry geom = world.geometry();
    world.placeNow(
        EntityFactory::createBasicZombie({geom.houseX() + 2.0F, geom.laneY(0)}, config));

    advance(world, 2.0F);

    EXPECT_EQ(world.playerHealth(), 80) << "exactly one hit, not one per step";
    EXPECT_EQ(countOf(world, EntityKind::BasicZombie), 0);
}

TEST(Combat, PeashooterFiresOnlyWhenAZombieIsAheadInItsLane) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;
    config.peashooter.actionInterval = 1.0F;

    World world = makeWorld(config);
    world.setSun(100);
    const GridCell cell{2, 1};

    sendCommand(world, SelectPlantCommand{PlantType::Peashooter});
    sendCommand(world, ClickCommand{world.geometry().cellCenter(cell)});

    advance(world, 2.5F);
    EXPECT_EQ(countOf(world, EntityKind::Projectile), 0) << "no target, no shots";

    const Entity* plant = findFirst(world, EntityKind::Peashooter);
    ASSERT_NE(plant, nullptr);
    world.placeNow(
        EntityFactory::createBasicZombie({plant->position().x + 200.0F, plant->position().y},
                                         config));

    advance(world, 1.5F);
    EXPECT_GT(countOf(world, EntityKind::Projectile), 0) << "target present, should fire";
}

TEST(Combat, PeashooterIgnoresZombiesBehindIt) {
    GameConfig config = quietConfig();
    config.basicZombie.speed = 0.0F;
    config.peashooter.actionInterval = 0.5F;

    World world = makeWorld(config);
    world.setSun(100);
    const GridCell cell{2, 5};

    sendCommand(world, SelectPlantCommand{PlantType::Peashooter});
    sendCommand(world, ClickCommand{world.geometry().cellCenter(cell)});
    advance(world, 0.05F);

    const Entity* plant = findFirst(world, EntityKind::Peashooter);
    ASSERT_NE(plant, nullptr);
    // Zombie already past the plant, walking away from it.
    world.placeNow(
        EntityFactory::createBasicZombie({plant->position().x - 150.0F, plant->position().y},
                                         config));

    advance(world, 2.0F);

    EXPECT_EQ(countOf(world, EntityKind::Projectile), 0);
}

} // namespace
} // namespace pvz
