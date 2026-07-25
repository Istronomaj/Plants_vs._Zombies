#pragma once

#include "pvz/core/Entity.hpp"
#include "pvz/core/GameConfig.hpp"

namespace pvz {

/// Walks right-to-left along one lane, eating any plant it meets.
class Zombie : public Entity {
public:
    Zombie(EntityKind kind, Vec2 position, const ZombieStats& stats, Vec2 hitboxSize) noexcept;

    void update(World& world, float dt) override;

    [[nodiscard]] int damage() const noexcept { return m_damage; }
    [[nodiscard]] float speed() const noexcept { return m_speed; }

    /// True while the zombie is stopped and eating.
    [[nodiscard]] bool isEating() const noexcept { return m_eating; }

private:
    /// Nearest plant this zombie's hitbox overlaps, or nullptr.
    [[nodiscard]] Entity* findTarget(World& world) const;

    float m_speed;
    int m_damage;
    float m_biteInterval;
    float m_biteCooldown;
    bool m_eating{false};
};

class BasicZombie final : public Zombie {
public:
    BasicZombie(Vec2 position, const GameConfig& config);
};

} // namespace pvz
