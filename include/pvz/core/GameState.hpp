#pragma once

#include <cstdint>
#include <string_view>

namespace pvz {

/// Simulation state.
///
/// Only states the simulation itself can be in. A title screen is a
/// presentation concern and lives in the app layer, because a World that has
/// not started yet would have no board.
enum class GameState : std::uint8_t {
    Playing,
    Paused,
    Won,
    Lost,
};

[[nodiscard]] constexpr bool isTerminal(GameState state) noexcept {
    return state == GameState::Won || state == GameState::Lost;
}

[[nodiscard]] constexpr std::string_view toString(GameState state) noexcept {
    switch (state) {
        case GameState::Playing:
            return "Playing";
        case GameState::Paused:
            return "Paused";
        case GameState::Won:
            return "Won";
        case GameState::Lost:
            return "Lost";
    }
    return "Unknown";
}

} // namespace pvz
