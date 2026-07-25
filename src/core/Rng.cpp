#include "pvz/core/Rng.hpp"

#include <algorithm>

namespace pvz {

Mt19937Rng Mt19937Rng::fromEntropy() {
    std::random_device device;
    return Mt19937Rng{device()};
}

int Mt19937Rng::nextInt(int lo, int hi) {
    if (lo > hi) {
        std::swap(lo, hi);
    }
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(m_gen);
}

float Mt19937Rng::nextFloat(float lo, float hi) {
    if (lo > hi) {
        std::swap(lo, hi);
    }
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(m_gen);
}

int SequenceRng::next() {
    if (m_values.empty()) {
        return 0;
    }
    const int value = m_values[m_index];
    m_index = (m_index + 1) % m_values.size();
    return value;
}

int SequenceRng::nextInt(int lo, int hi) {
    if (lo > hi) {
        std::swap(lo, hi);
    }
    return std::clamp(next(), lo, hi);
}

float SequenceRng::nextFloat(float lo, float hi) {
    if (lo > hi) {
        std::swap(lo, hi);
    }
    return std::clamp(static_cast<float>(next()), lo, hi);
}

} // namespace pvz
