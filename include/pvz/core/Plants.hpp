#pragma once

#include "pvz/core/Plant.hpp"

namespace pvz {

/// Fires a pea down its lane whenever a zombie is present in that lane.
class Peashooter final : public Plant {
public:
    Peashooter(Vec2 position, GridCell cell, const GameConfig& config);

protected:
    void action(World& world) override;

private:
    int m_damage;
    float m_projectileSpeed;
    float m_spawnOffset;
};

/// Periodically produces a sun that drifts to a nearby point.
class Sunflower final : public Plant {
public:
    Sunflower(Vec2 position, GridCell cell, const GameConfig& config);

protected:
    void action(World& world) override;

private:
    float m_scatterRadius;
};

/// Pure blocker: high health, no action. The original gave it a damage value
/// and a cooldown copied from Peashooter despite its action() being empty.
class Wallnut final : public Plant {
public:
    Wallnut(Vec2 position, GridCell cell, const GameConfig& config);

protected:
    void action(World& world) override;
};

} // namespace pvz
