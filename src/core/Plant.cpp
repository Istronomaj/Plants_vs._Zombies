#include "pvz/core/Plant.hpp"

namespace pvz {

Plant::Plant(EntityKind kind, Vec2 position, GridCell cell, const PlantStats& stats,
             Vec2 hitboxSize) noexcept
    : Entity(kind, position, stats.health, hitboxSize),
      m_cell(cell),
      m_cost(stats.cost),
      m_actionInterval(stats.actionInterval),
      m_cooldown(stats.actionInterval) {}

void Plant::update(World& world, float dt) {
    if (!hasAction()) {
        return;
    }

    m_cooldown -= dt;
    // `+=` rather than `= interval` so leftover time carries into the next
    // period; assigning would discard the remainder every cycle and make
    // production drift slower than configured. The loop covers dt > interval.
    while (m_cooldown <= 0.0F) {
        m_cooldown += m_actionInterval;
        action(world);
    }
}

} // namespace pvz
