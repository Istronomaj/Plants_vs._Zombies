#pragma once

#include "pvz/core/Vec2.hpp"

namespace pvz {

/// Axis-aligned bounding box.
///
/// `position` is the TOP-LEFT corner, matching sf::FloatRect so the app layer
/// can convert without re-deriving the convention. Entities store a centre
/// point, so `Rect::fromCenter` is the intended way to build one.
struct Rect {
    Vec2 position{};
    Vec2 size{};

    [[nodiscard]] static constexpr Rect fromCenter(Vec2 center, Vec2 size) noexcept {
        return {{center.x - size.x / 2.0F, center.y - size.y / 2.0F}, size};
    }

    [[nodiscard]] constexpr Vec2 center() const noexcept {
        return {position.x + size.x / 2.0F, position.y + size.y / 2.0F};
    }

    [[nodiscard]] constexpr float left() const noexcept { return position.x; }
    [[nodiscard]] constexpr float top() const noexcept { return position.y; }
    [[nodiscard]] constexpr float right() const noexcept { return position.x + size.x; }
    [[nodiscard]] constexpr float bottom() const noexcept { return position.y + size.y; }

    [[nodiscard]] constexpr bool contains(Vec2 point) const noexcept {
        return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const noexcept {
        return left() < other.right() && other.left() < right() && top() < other.bottom() &&
               other.top() < bottom();
    }
};

} // namespace pvz
