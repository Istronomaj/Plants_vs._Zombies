#pragma once

#include "pvz/core/EntityKind.hpp"
#include "pvz/core/Rect.hpp"
#include "pvz/core/Vec2.hpp"

namespace pvz {

class World;

/// Base class for everything that lives on the lawn.
///
/// Unlike the original Entity, this has no graphics dependency: collision uses
/// the project's own Rect rather than an sf::RectangleShape, so the whole
/// simulation can run and be tested without a window.
///
/// All five special members are defaulted. The original hand-wrote a copy
/// constructor and copy assignment it did not need, and both were wrong: the
/// copy constructor initialised maxTicks from its own just-initialised ticks
/// member instead of the source's, and operator= silently skipped five of
/// eleven members. Declaring them also suppressed move generation.
class Entity {
public:
    Entity(EntityKind kind, Vec2 position, int health, Vec2 hitboxSize) noexcept;

    Entity(const Entity&) = default;
    Entity& operator=(const Entity&) = default;
    Entity(Entity&&) noexcept = default;
    Entity& operator=(Entity&&) noexcept = default;
    virtual ~Entity() = default;

    /// Advances this entity by `dt` seconds.
    virtual void update(World& world, float dt) = 0;

    /// Applies damage, killing the entity if health reaches zero.
    void takeDamage(int amount) noexcept;

    [[nodiscard]] EntityKind kind() const noexcept { return m_kind; }
    [[nodiscard]] EntityCategory category() const noexcept { return categoryOf(m_kind); }
    [[nodiscard]] Vec2 position() const noexcept { return m_position; }
    [[nodiscard]] Vec2 hitboxSize() const noexcept { return m_hitboxSize; }
    [[nodiscard]] int health() const noexcept { return m_health; }
    [[nodiscard]] bool isAlive() const noexcept { return m_alive; }
    [[nodiscard]] int renderLayer() const noexcept { return renderLayerOf(m_kind); }

    [[nodiscard]] Rect bounds() const noexcept {
        return Rect::fromCenter(m_position, m_hitboxSize);
    }

    /// Marks the entity for removal at the end of the current step.
    void kill() noexcept { m_alive = false; }

protected:
    Vec2 m_position;
    Vec2 m_hitboxSize;
    int m_health;
    bool m_alive{true};

private:
    EntityKind m_kind;
};

} // namespace pvz
