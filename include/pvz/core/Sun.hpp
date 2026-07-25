#pragma once

#include "pvz/core/Entity.hpp"

namespace pvz {

/// Collectable sun. Drifts to a target point, then waits to be clicked.
///
/// Unlike the original, sun expires: uncollected sun used to accumulate on the
/// lawn forever.
class Sun final : public Entity {
public:
    Sun(Vec2 position, Vec2 target, float speed, float lifetime, Vec2 hitboxSize) noexcept;

    void update(World& world, float dt) override;

    [[nodiscard]] float remainingLifetime() const noexcept { return m_lifetime; }

private:
    Vec2 m_target;
    float m_speed;
    float m_lifetime;
};

} // namespace pvz
