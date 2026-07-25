#include "pvz/core/Sun.hpp"

#include "pvz/core/World.hpp"

namespace pvz {

Sun::Sun(Vec2 position, Vec2 target, float speed, float lifetime, Vec2 hitboxSize) noexcept
    : Entity(EntityKind::Sun, position, 1, hitboxSize),
      m_target(target),
      m_speed(speed),
      m_lifetime(lifetime) {}

void Sun::update(World& /*world*/, float dt) {
    m_lifetime -= dt;
    if (m_lifetime <= 0.0F) {
        kill();
        return;
    }

    const Vec2 toTarget = m_target - m_position;
    const float distance = length(toTarget);
    const float travel = m_speed * dt;

    if (distance <= travel) {
        m_position = m_target;  // snap, so it never oscillates around the target
        return;
    }

    m_position += normalized(toTarget) * travel;
}

} // namespace pvz
