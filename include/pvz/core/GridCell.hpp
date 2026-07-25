#pragma once

namespace pvz {

/// A lawn tile address. Row 0 is the top lane, column 0 the leftmost.
struct GridCell {
    int row{};
    int col{};

    friend constexpr bool operator==(GridCell, GridCell) noexcept = default;
};

} // namespace pvz
