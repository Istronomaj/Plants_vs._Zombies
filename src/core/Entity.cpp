#include "pvz/core/Entity.hpp"

namespace pvz {

Entity::Entity(EntityKind kind, Vec2 position, int health, Vec2 hitboxSize) noexcept
    : m_position(position), m_hitboxSize(hitboxSize), m_health(health), m_kind(kind) {}

void Entity::takeDamage(int amount) noexcept {
    if (amount <= 0) {
        return;
    }
    m_health -= amount;
    if (m_health <= 0) {
        m_health = 0;
        m_alive = false;
    }
}

} // namespace pvz
