#pragma once

#include "pvz/core/GridCell.hpp"

#include <vector>

namespace pvz {

/// Tile occupancy for the lawn.
///
/// Occupancy used to live in a per-Grid copy of a mutable global settings map,
/// where the "is this tile taken" flag was the second element of a pair inside
/// a string-keyed map of vectors. Here it is just a grid of bools.
class Board {
public:
    Board(int rows, int cols);

    [[nodiscard]] bool inBounds(GridCell cell) const noexcept;
    [[nodiscard]] bool isOccupied(GridCell cell) const noexcept;

    void occupy(GridCell cell) noexcept;
    void free(GridCell cell) noexcept;
    void clear() noexcept;

    [[nodiscard]] int rows() const noexcept { return m_rows; }
    [[nodiscard]] int cols() const noexcept { return m_cols; }

private:
    [[nodiscard]] std::size_t indexOf(GridCell cell) const noexcept;

    int m_rows;
    int m_cols;
    std::vector<bool> m_occupied;
};

} // namespace pvz
