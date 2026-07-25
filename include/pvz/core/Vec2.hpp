#pragma once

#include <cmath>

namespace pvz {

/// A 2D vector in world (map) space.
///
/// Deliberately float rather than int: movement is delta-time based, so
/// entities routinely advance by sub-pixel amounts per simulation step.
struct Vec2 {
    float x{};
    float y{};

    constexpr Vec2& operator+=(Vec2 other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2& operator-=(Vec2 other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    friend constexpr Vec2 operator+(Vec2 a, Vec2 b) noexcept { return {a.x + b.x, a.y + b.y}; }

    friend constexpr Vec2 operator-(Vec2 a, Vec2 b) noexcept { return {a.x - b.x, a.y - b.y}; }

    friend constexpr Vec2 operator*(Vec2 v, float s) noexcept { return {v.x * s, v.y * s}; }

    friend constexpr Vec2 operator*(float s, Vec2 v) noexcept { return v * s; }

    friend constexpr Vec2 operator-(Vec2 v) noexcept { return {-v.x, -v.y}; }

    friend constexpr bool operator==(Vec2, Vec2) noexcept = default;
};

[[nodiscard]] constexpr float lengthSquared(Vec2 v) noexcept {
    return v.x * v.x + v.y * v.y;
}

[[nodiscard]] inline float length(Vec2 v) noexcept {
    return std::sqrt(lengthSquared(v));
}

/// Unit vector in the same direction, or {0, 0} if `v` is (near) zero.
[[nodiscard]] inline Vec2 normalized(Vec2 v) noexcept {
    const float len = length(v);
    if (len <= 1e-6F) {
        return {};
    }
    return {v.x / len, v.y / len};
}

} // namespace pvz
