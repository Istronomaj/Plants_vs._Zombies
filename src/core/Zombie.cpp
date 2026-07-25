#include "pvz/core/Zombie.hpp"

#include "pvz/core/World.hpp"

namespace pvz {

Zombie::Zombie(EntityKind kind, Vec2 position, const ZombieStats& stats, Vec2 hitboxSize) noexcept
    : Entity(kind, position, stats.health, hitboxSize),
      m_speed(stats.speed),
      m_damage(stats.damage),
      m_biteInterval(stats.biteInterval),
      m_biteCooldown(stats.biteInterval) {}

Entity* Zombie::findTarget(World& world) const {
    const Rect self = bounds();
    Entity* best = nullptr;
    for (Entity* plant : world.plants()) {
        if (!plant->isAlive() || !self.intersects(plant->bounds())) {
            continue;
        }
        // Eat the rightmost overlapping plant, i.e. the one reached first.
        if (best == nullptr || plant->position().x > best->position().x) {
            best = plant;
        }
    }
    return best;
}

void Zombie::update(World& world, float dt) {
    // Reaching the house damages the player once, then the zombie is gone.
    // The original looped over five hard-coded hit points and could both apply
    // damage several times in one frame and, via an `else` inside that loop,
    // un-block a zombie that was in the middle of eating.
    if (m_position.x <= world.geometry().houseX()) {
        world.damagePlayer(m_damage);
        kill();
        return;
    }

    Entity* target = findTarget(world);
    m_eating = target != nullptr;

    if (!m_eating) {
        m_biteCooldown = m_biteInterval;
        m_position.x -= m_speed * dt;
        return;
    }

    // Stopped and biting: the first bite lands one full interval after
    // contact, and leftover time carries over so the rate stays exact.
    m_biteCooldown -= dt;
    while (m_biteCooldown <= 0.0F && target->isAlive()) {
        m_biteCooldown += m_biteInterval;
        target->takeDamage(m_damage);
    }
}

BasicZombie::BasicZombie(Vec2 position, const GameConfig& config)
    : Zombie(EntityKind::BasicZombie, position, config.basicZombie, config.defaultHitboxSize) {}

} // namespace pvz
