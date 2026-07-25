#include "TestHelpers.hpp"

#include "pvz/core/World.hpp"

#include <gtest/gtest.h>

namespace pvz {
namespace {

using test::advance;
using test::countOf;
using test::makeWorld;
using test::quietConfig;
using test::sendCommand;

Command clickOn(const World& world, GridCell cell) {
    return ClickCommand{world.geometry().cellCenter(cell)};
}

/// The original charged the sun cost inside Grid::addEntity, which returned
/// void and simply bailed when the player could not afford the plant -- while
/// the caller marked the tile occupied regardless. Clicking a tile without
/// enough sun therefore burned that tile for the rest of the match.
TEST(Planting, InsufficientSunLeavesTilePlantable) {
    World world = makeWorld();
    const GridCell cell{2, 3};

    world.setSun(0);
    sendCommand(world, SelectPlantCommand{PlantType::Peashooter});
    sendCommand(world, clickOn(world, cell));

    EXPECT_FALSE(world.board().isOccupied(cell)) << "a failed purchase must not consume the tile";
    EXPECT_EQ(countOf(world, EntityKind::Peashooter), 0);

    // The same tile must still work once the player can afford it.
    world.setSun(world.config().peashooter.cost);
    sendCommand(world, clickOn(world, cell));

    EXPECT_TRUE(world.board().isOccupied(cell));
    EXPECT_EQ(countOf(world, EntityKind::Peashooter), 1);
    EXPECT_EQ(world.sun(), 0);
}

TEST(Planting, SuccessfulPlantDeductsExactlyTheCost) {
    World world = makeWorld();
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Sunflower});
    sendCommand(world, clickOn(world, {0, 0}));

    EXPECT_EQ(world.sun(), 100 - world.config().sunflower.cost);
}

TEST(Planting, OccupiedTileRejectsSecondPlantAndKeepsSun) {
    World world = makeWorld();
    const GridCell cell{1, 1};
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Peashooter});
    sendCommand(world, clickOn(world, cell));
    const int sunAfterFirst = world.sun();

    sendCommand(world, clickOn(world, cell));

    EXPECT_EQ(world.sun(), sunAfterFirst) << "a rejected plant must not be charged";
    EXPECT_EQ(countOf(world, EntityKind::Peashooter), 1);
}

TEST(Planting, TileIsFreedWhenThePlantDies) {
    World world = makeWorld();
    const GridCell cell{3, 4};
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Wallnut});
    sendCommand(world, clickOn(world, cell));
    ASSERT_TRUE(world.board().isOccupied(cell));

    Entity* plant = test::findFirst(world, EntityKind::Wallnut);
    ASSERT_NE(plant, nullptr);
    plant->takeDamage(world.config().wallnut.health);

    advance(world, 0.1F);

    EXPECT_FALSE(world.board().isOccupied(cell)) << "the tile must be replantable";

    sendCommand(world, clickOn(world, cell));
    EXPECT_EQ(countOf(world, EntityKind::Wallnut), 1);
}

TEST(Planting, ClickOutsideTheLawnDoesNothing) {
    World world = makeWorld();
    world.setSun(100);
    const int sunBefore = world.sun();

    sendCommand(world, ClickCommand{{-500.0F, -500.0F}});

    EXPECT_EQ(world.sun(), sunBefore);
    EXPECT_EQ(countOf(world, EntityKind::Peashooter), 0);
}

TEST(Planting, SelectedPlantDeterminesWhatIsPlaced) {
    World world = makeWorld();
    world.setSun(100);

    sendCommand(world, SelectPlantCommand{PlantType::Sunflower});
    EXPECT_EQ(world.selectedPlant(), PlantType::Sunflower);
    sendCommand(world, clickOn(world, {0, 0}));

    sendCommand(world, SelectPlantCommand{PlantType::Wallnut});
    sendCommand(world, clickOn(world, {0, 1}));

    EXPECT_EQ(countOf(world, EntityKind::Sunflower), 1);
    EXPECT_EQ(countOf(world, EntityKind::Wallnut), 1);
    EXPECT_EQ(countOf(world, EntityKind::Peashooter), 0);
}

TEST(Planting, PlantIsPositionedAtItsTileCentre) {
    World world = makeWorld();
    const GridCell cell{2, 5};
    world.setSun(100);

    sendCommand(world, clickOn(world, cell));

    const Entity* plant = test::findFirst(world, EntityKind::Peashooter);
    ASSERT_NE(plant, nullptr);
    EXPECT_EQ(plant->position(), world.geometry().cellCenter(cell));
}

} // namespace
} // namespace pvz
