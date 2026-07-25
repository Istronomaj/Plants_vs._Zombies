#include "pvz/core/Board.hpp"

namespace pvz {

Board::Board(int rows, int cols)
    : m_rows(rows > 0 ? rows : 0),
      m_cols(cols > 0 ? cols : 0),
      m_occupied(static_cast<std::size_t>(m_rows) * static_cast<std::size_t>(m_cols), false) {}

std::size_t Board::indexOf(GridCell cell) const noexcept {
    return static_cast<std::size_t>(cell.row) * static_cast<std::size_t>(m_cols) +
           static_cast<std::size_t>(cell.col);
}

bool Board::inBounds(GridCell cell) const noexcept {
    return cell.row >= 0 && cell.row < m_rows && cell.col >= 0 && cell.col < m_cols;
}

bool Board::isOccupied(GridCell cell) const noexcept {
    if (!inBounds(cell)) {
        return false;
    }
    return m_occupied[indexOf(cell)];
}

void Board::occupy(GridCell cell) noexcept {
    if (inBounds(cell)) {
        m_occupied[indexOf(cell)] = true;
    }
}

void Board::free(GridCell cell) noexcept {
    if (inBounds(cell)) {
        m_occupied[indexOf(cell)] = false;
    }
}

void Board::clear() noexcept {
    m_occupied.assign(m_occupied.size(), false);
}

} // namespace pvz
