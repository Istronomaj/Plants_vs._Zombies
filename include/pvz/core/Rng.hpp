#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace pvz {

/// Random number source.
///
/// An interface rather than a free function so the simulation can be driven
/// deterministically from tests. The previous implementation constructed a
/// std::random_device AND a fresh std::mt19937 on every single call, which is
/// both slow and, on some MinGW toolchains, not actually random.
class Rng {
public:
    Rng() = default;
    Rng(const Rng&) = default;
    Rng& operator=(const Rng&) = default;
    Rng(Rng&&) noexcept = default;
    Rng& operator=(Rng&&) noexcept = default;
    virtual ~Rng() = default;

    /// Uniform integer in [lo, hi] (inclusive on both ends).
    [[nodiscard]] virtual int nextInt(int lo, int hi) = 0;

    /// Uniform float in [lo, hi).
    [[nodiscard]] virtual float nextFloat(float lo, float hi) = 0;
};

/// Mersenne Twister, seeded exactly once at construction.
class Mt19937Rng final : public Rng {
public:
    explicit Mt19937Rng(std::uint32_t seed) : m_gen(seed) {}

    [[nodiscard]] static Mt19937Rng fromEntropy();

    [[nodiscard]] int nextInt(int lo, int hi) override;
    [[nodiscard]] float nextFloat(float lo, float hi) override;

private:
    std::mt19937 m_gen;
};

/// Replays a fixed sequence, cycling when exhausted. Tests only.
///
/// Lets a test state "the next spawn lane is exactly 2" instead of hoping a
/// seeded generator happens to produce it.
class SequenceRng final : public Rng {
public:
    explicit SequenceRng(std::vector<int> values) : m_values(std::move(values)) {}

    [[nodiscard]] int nextInt(int lo, int hi) override;
    [[nodiscard]] float nextFloat(float lo, float hi) override;

private:
    [[nodiscard]] int next();

    std::vector<int> m_values;
    std::size_t m_index{0};
};

} // namespace pvz
