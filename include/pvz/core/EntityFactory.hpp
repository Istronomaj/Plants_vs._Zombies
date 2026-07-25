#pragma once

#include "pvz/core/Entity.hpp"
#include "pvz/core/GameConfig.hpp"
#include "pvz/core/GridCell.hpp"

#include <memory>

namespace pvz {

namespace EntityFactory {

[[nodiscard]] std::unique_ptr<Entity> createPlant(PlantType type, Vec2 position, GridCell cell,
                                                 const GameConfig& config);

[[nodiscard]] std::unique_ptr<Entity> createBasicZombie(Vec2 position, const GameConfig& config);

[[nodiscard]] std::unique_ptr<Entity> createProjectile(Vec2 position, int damage, float speed,
                                                       const GameConfig& config);

[[nodiscard]] std::unique_ptr<Entity> createSun(Vec2 position, Vec2 target,
                                                const GameConfig& config);

} // namespace EntityFactory

} // namespace pvz
