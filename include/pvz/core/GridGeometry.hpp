#pragma once

#include "pvz/core/GridCell.hpp"
#include "pvz/core/Vec2.hpp"

#include <optional>

namespace pvz {

/// Maps between lawn tiles and world positions.
///
/// Replaces the 45 hand-typed pixel coordinates the original stored in a
/// mutable global `std::unordered_map<std::string, vector<pair<Vector2i,bool>>>`.
/// Those coordinates were near-uniform; a least-squares fit reproduces every
/// one of them to within 4px, well inside the click tolerance.
class GridGeometry {
public:
    constexpr GridGeometry(Vec2 origin, Vec2 cellSize, int rows, int cols, float houseX,
                           float spawnX) noexcept
        : m_origin(origin),
          m_cellSize(cellSize),
          m_rows(rows),
          m_cols(cols),
          m_houseX(houseX),
          m_spawnX(spawnX) {}

    /// Centre of the given tile. `origin` is the centre of cell (0, 0).
    [[nodiscard]] constexpr Vec2 cellCenter(GridCell cell) const noexcept {
        return {m_origin.x + static_cast<float>(cell.col) * m_cellSize.x,
                m_origin.y + static_cast<float>(cell.row) * m_cellSize.y};
    }

    [[nodiscard]] constexpr float laneY(int row) const noexcept {
        return m_origin.y + static_cast<float>(row) * m_cellSize.y;
    }

    /// Which tile contains `point`, or nullopt if it is off the lawn.
    [[nodiscard]] constexpr std::optional<GridCell> cellAt(Vec2 point) const noexcept {
        // +0.5 because origin is a cell CENTRE, so the tile spans [-0.5, +0.5)
        // cells around it. The negative guard matters: truncating -0.3 yields 0,
        // which would wrongly report a click left of the lawn as column 0.
        const float col = (point.x - m_origin.x) / m_cellSize.x + 0.5F;
        const float row = (point.y - m_origin.y) / m_cellSize.y + 0.5F;
        if (col < 0.0F || row < 0.0F) {
            return std::nullopt;
        }
        const GridCell cell{static_cast<int>(row), static_cast<int>(col)};
        if (cell.row >= m_rows || cell.col >= m_cols) {
            return std::nullopt;
        }
        return cell;
    }

    [[nodiscard]] constexpr bool inBounds(GridCell cell) const noexcept {
        return cell.row >= 0 && cell.row < m_rows && cell.col >= 0 && cell.col < m_cols;
    }

    /// X coordinate at which a zombie is considered to have reached the house.
    [[nodiscard]] constexpr float houseX() const noexcept { return m_houseX; }

    /// X coordinate at which zombies enter the lawn.
    [[nodiscard]] constexpr float spawnX() const noexcept { return m_spawnX; }

    [[nodiscard]] constexpr int rows() const noexcept { return m_rows; }
    [[nodiscard]] constexpr int cols() const noexcept { return m_cols; }
    [[nodiscard]] constexpr Vec2 origin() const noexcept { return m_origin; }
    [[nodiscard]] constexpr Vec2 cellSize() const noexcept { return m_cellSize; }

private:
    Vec2 m_origin;
    Vec2 m_cellSize;
    int m_rows;
    int m_cols;
    float m_houseX;
    float m_spawnX;
};

} // namespace pvz
