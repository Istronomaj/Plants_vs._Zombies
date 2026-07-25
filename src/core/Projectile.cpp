#include "pvz/core/Projectile.hpp"

#include "pvz/core/World.hpp"

namespace pvz {

Projectile::Projectile(Vec2 position, int damage, float speed, Vec2 hitboxSize) noexcept
    : Entity(EntityKind::Projectile, position, 1, hitboxSize),
      m_damage(damage),
      m_speed(speed) {}

void Projectile::update(World& world, float dt) {
    m_position.x += m_speed * dt;

    // Off the right edge of the lawn: despawn rather than fly forever.
    if (m_position.x > world.geometry().spawnX()) {
        kill();
        return;
    }

    const Rect self = bounds();
    const int lane = world.laneOf(m_position.y);

    // Hits the leftmost zombie it overlaps, then stops. The original had no
    // break, so one pea damaged every overlapping zombie at once.
    Entity* hit = nullptr;
    for (Entity* zombie : world.zombiesInLane(lane)) {
        if (!zombie->isAlive() || !self.intersects(zombie->bounds())) {
            continue;
        }
        if (hit == nullptr || zombie->position().x < hit->position().x) {
            hit = zombie;
        }
    }

    if (hit != nullptr) {
        hit->takeDamage(m_damage);
        kill();
    }
}

} // namespace pvz
