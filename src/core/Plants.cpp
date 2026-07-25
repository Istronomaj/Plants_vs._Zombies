#include "pvz/core/Plants.hpp"

#include "pvz/core/EntityFactory.hpp"
#include "pvz/core/World.hpp"

namespace pvz {

// --- Peashooter ---

Peashooter::Peashooter(Vec2 position, GridCell cell, const GameConfig& config)
    : Plant(EntityKind::Peashooter, position, cell, config.peashooter, config.defaultHitboxSize),
      m_damage(config.peashooter.damage),
      m_projectileSpeed(config.peashooter.projectileSpeed),
      m_spawnOffset(config.projectileSpawnOffset) {}

void Peashooter::action(World& world) {
    // Only zombies in this plant's own lane matter, and only those still ahead
    // of it. The original scanned every entity in the game with a dynamic_cast
    // and compared lane by exact integer Y equality.
    const int lane = world.laneOf(m_position.y);
    bool targetAhead = false;
    for (const Entity* zombie : world.zombiesInLane(lane)) {
        if (zombie->position().x >= m_position.x) {
            targetAhead = true;
            break;
        }
    }
    if (!targetAhead) {
        return;
    }

    world.spawn(EntityFactory::createProjectile({m_position.x + m_spawnOffset, m_position.y},
                                                m_damage, m_projectileSpeed, world.config()));
}

// --- Sunflower ---

Sunflower::Sunflower(Vec2 position, GridCell cell, const GameConfig& config)
    : Plant(EntityKind::Sunflower, position, cell, config.sunflower, config.defaultHitboxSize),
      m_scatterRadius(config.sunScatterRadius) {}

void Sunflower::action(World& world) {
    const Vec2 target{m_position.x + world.rng().nextFloat(-m_scatterRadius, m_scatterRadius),
                      m_position.y + world.rng().nextFloat(-m_scatterRadius, m_scatterRadius)};
    world.spawn(EntityFactory::createSun(m_position, target, world.config()));
}

// --- Wallnut ---

Wallnut::Wallnut(Vec2 position, GridCell cell, const GameConfig& config)
    : Plant(EntityKind::Wallnut, position, cell, config.wallnut, config.defaultHitboxSize) {}

void Wallnut::action(World& /*world*/) {
    // Intentionally empty: a wallnut only absorbs damage. Its action interval
    // is zero, so Plant::update never calls this.
}

} // namespace pvz
