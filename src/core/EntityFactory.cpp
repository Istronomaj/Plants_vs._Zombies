#include "pvz/core/EntityFactory.hpp"

#include "pvz/core/Plants.hpp"
#include "pvz/core/Projectile.hpp"
#include "pvz/core/Sun.hpp"
#include "pvz/core/Zombie.hpp"

namespace pvz {

std::unique_ptr<Entity> EntityFactory::createPlant(PlantType type, Vec2 position, GridCell cell,
                                                   const GameConfig& config) {
    switch (type) {
        case PlantType::Peashooter:
            return std::make_unique<Peashooter>(position, cell, config);
        case PlantType::Wallnut:
            return std::make_unique<Wallnut>(position, cell, config);
        case PlantType::Sunflower:
            return std::make_unique<Sunflower>(position, cell, config);
    }
    return std::make_unique<Peashooter>(position, cell, config);
}

std::unique_ptr<Entity> EntityFactory::createBasicZombie(Vec2 position, const GameConfig& config) {
    return std::make_unique<BasicZombie>(position, config);
}

std::unique_ptr<Entity> EntityFactory::createProjectile(Vec2 position, int damage, float speed,
                                                        const GameConfig& config) {
    return std::make_unique<Projectile>(position, damage, speed, config.projectileHitboxSize);
}

std::unique_ptr<Entity> EntityFactory::createSun(Vec2 position, Vec2 target,
                                                 const GameConfig& config) {
    return std::make_unique<Sun>(position, target, config.sunDriftSpeed, config.sunLifetime,
                                 config.sunHitboxSize);
}

} // namespace pvz
