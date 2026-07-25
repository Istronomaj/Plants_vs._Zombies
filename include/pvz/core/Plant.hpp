#pragma once

#include "pvz/core/Entity.hpp"
#include "pvz/core/GameConfig.hpp"
#include "pvz/core/GridCell.hpp"

namespace pvz {

/// A plant occupying one lawn tile.
///
/// Stores the cell it was planted on so the board can be freed on death.
/// The original compared the plant's position against the coordinate table
/// with integer equality, which is fragile when positions come from floats.
class Plant : public Entity {
public:
    Plant(EntityKind kind, Vec2 position, GridCell cell, const PlantStats& stats,
          Vec2 hitboxSize) noexcept;

    void update(World& world, float dt) final;

    [[nodiscard]] int cost() const noexcept { return m_cost; }
    [[nodiscard]] GridCell cell() const noexcept { return m_cell; }

    /// Wallnut has no periodic action; its interval is zero.
    [[nodiscard]] bool hasAction() const noexcept { return m_actionInterval > 0.0F; }

protected:
    /// Called once per elapsed action interval.
    virtual void action(World& world) = 0;

private:
    GridCell m_cell;
    int m_cost;
    float m_actionInterval;
    float m_cooldown;
};

} // namespace pvz
