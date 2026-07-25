#pragma once

#include "pvz/core/EntityKind.hpp"
#include "pvz/core/Vec2.hpp"

#include <variant>

namespace pvz {

/// A left-click at a world position.
///
/// Deliberately one command rather than separate "collect sun" and "plant"
/// commands: which of the two a click means is a gameplay rule (sun wins), so
/// it belongs in the simulation where it can be tested, not in the input layer.
struct ClickCommand {
    Vec2 worldPosition;
};

struct SelectPlantCommand {
    PlantType type;
};

struct PauseCommand {};

struct ResumeCommand {};

struct RestartCommand {};

/// Player input, expressed without reference to any windowing library. Tests
/// drive the simulation by pushing these directly into World::step.
using Command = std::variant<ClickCommand, SelectPlantCommand, PauseCommand, ResumeCommand,
                             RestartCommand>;

} // namespace pvz
