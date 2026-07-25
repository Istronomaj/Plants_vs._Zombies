#pragma once

#include "pvz/core/Entity.hpp"

namespace pvz {

/// A pea travelling left-to-right, damaging the first zombie it touches.
class Projectile final : public Entity {
public:
    Projectile(Vec2 position, int damage, float speed, Vec2 hitboxSize) noexcept;

    void update(World& world, float dt) override;

    [[nodiscard]] int damage() const noexcept { return m_damage; }

private:
    int m_damage;
    float m_speed;
};

} // namespace pvz
