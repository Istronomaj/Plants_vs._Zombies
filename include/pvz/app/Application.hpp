#pragma once

#include "pvz/app/GameRenderer.hpp"
#include "pvz/app/HudRenderer.hpp"
#include "pvz/app/ResourceManager.hpp"
#include "pvz/app/ScreenRenderer.hpp"
#include "pvz/core/Commands.hpp"
#include "pvz/core/GameConfig.hpp"
#include "pvz/core/World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace pvz::app {

/// Which part of the game the player is looking at.
///
/// Distinct from GameState: the simulation knows about playing, paused, won
/// and lost, but a title screen is purely presentational -- there is no World
/// behind it.
enum class Screen : std::uint8_t {
    MainMenu,
    InGame,
};

/// Command-line options.
struct AppOptions {
    std::filesystem::path assetDirectory;
    std::filesystem::path balanceFile;
    /// Window height as a multiple of the map's native size.
    float scale = 2.0F;
    std::optional<std::uint32_t> seed;
};

[[nodiscard]] AppOptions parseArgs(int argc, char** argv);

/// Owns the window, the resources and the simulation.
///
/// The original had this inverted: a Renderer constructor called create() on a
/// window owned by a Game singleton, so the window only became real partway
/// through Game::run().
class Application {
public:
    explicit Application(AppOptions options);

    /// Runs until the window closes. Returns the process exit code.
    int run();

private:
    void processEvents();
    void handleKeyPressed(const sf::Event::KeyPressed& key);
    void handleMousePressed(const sf::Event::MouseButtonPressed& mouse);
    void startNewGame();
    void render();

    AppOptions m_options;
    GameConfig m_config;
    ResourceManager m_resources;

    sf::RenderWindow m_window;
    ViewTransform m_view;

    // Constructed after the window exists, since they size themselves from it
    // and hold SFML resources that cannot be default-constructed.
    std::optional<GameRenderer> m_gameRenderer;
    std::optional<HudRenderer> m_hud;
    std::optional<ScreenRenderer> m_screens;

    std::optional<World> m_world;
    Screen m_screen{Screen::MainMenu};
    std::vector<Command> m_commands;
    sf::Clock m_clock;
};

} // namespace pvz::app
