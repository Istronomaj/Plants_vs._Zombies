#include "pvz/app/Application.hpp"

#include "pvz/app/AssetPaths.hpp"
#include "pvz/core/Rng.hpp"

#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace pvz::app {
namespace {

constexpr const char* kWindowTitle = "Plants vs. Zombies";

[[nodiscard]] GameConfig loadConfig(const std::filesystem::path& assetDir,
                                    const std::filesystem::path& explicitFile) {
    const std::filesystem::path path =
        explicitFile.empty() ? assetDir / "balance.toml" : explicitFile;

    std::string error;
    if (auto loaded = GameConfig::loadFromToml(path, error)) {
        std::cout << "Loaded balance overrides from " << path.string() << '\n';
        return *loaded;
    }

    // A missing or broken balance file must never stop the game starting.
    // Only complain when the user asked for a specific file.
    if (!explicitFile.empty()) {
        std::cerr << "Warning: " << error << "\nFalling back to built-in defaults.\n";
    }
    return GameConfig{};
}

} // namespace

AppOptions parseArgs(int argc, char** argv) {
    AppOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const bool hasNext = (i + 1) < argc;

        if (arg == "--assets" && hasNext) {
            options.assetDirectory = argv[++i];
        } else if (arg == "--balance" && hasNext) {
            options.balanceFile = argv[++i];
        } else if (arg == "--scale" && hasNext) {
            options.scale = std::clamp(std::stof(argv[++i]), 0.5F, 4.0F);
        } else if (arg == "--seed" && hasNext) {
            options.seed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Plants vs. Zombies\n\n"
                         "  --assets <dir>    directory containing the game assets\n"
                         "  --balance <file>  TOML file overriding the balance values\n"
                         "  --scale <factor>  window scale (0.5 - 4.0, default 2.0)\n"
                         "  --seed <number>   fixed RNG seed, for reproducible runs\n";
            std::exit(0);
        }
    }
    return options;
}

Application::Application(AppOptions options)
    : m_options(std::move(options)),
      m_config(loadConfig(resolveAssetDirectory(m_options.assetDirectory), m_options.balanceFile)),
      m_resources(resolveAssetDirectory(m_options.assetDirectory)),
      m_view(m_options.scale) {
    // Eager, so a missing asset is reported by name at startup rather than
    // throwing out of a draw call later.
    m_resources.loadAll();

    const sf::Vector2u mapSize = m_resources.texture(TextureId::Map).getSize();
    const sf::Vector2u windowSize{
        static_cast<unsigned int>(static_cast<float>(mapSize.x) * m_options.scale),
        static_cast<unsigned int>(static_cast<float>(mapSize.y) * m_options.scale)};

    m_window.create(sf::VideoMode{windowSize}, kWindowTitle, sf::Style::Close);
    m_window.setVerticalSyncEnabled(true);

    m_gameRenderer.emplace(m_resources, m_view);
    m_hud.emplace(m_resources, windowSize);
    m_screens.emplace(m_resources, windowSize);
}

void Application::startNewGame() {
    auto rng = m_options.seed ? std::make_unique<Mt19937Rng>(*m_options.seed)
                              : std::make_unique<Mt19937Rng>(Mt19937Rng::fromEntropy());
    m_world.emplace(m_config, std::move(rng));
    m_screen = Screen::InGame;
}

void Application::handleKeyPressed(const sf::Event::KeyPressed& key) {
    using Key = sf::Keyboard::Key;

    if (m_screen == Screen::MainMenu) {
        if (key.code == Key::Enter || key.code == Key::Space) {
            startNewGame();
        } else if (key.code == Key::Escape) {
            m_window.close();
        }
        return;
    }

    switch (key.code) {
        case Key::Num1:
        case Key::Numpad1:
            m_commands.emplace_back(SelectPlantCommand{PlantType::Peashooter});
            break;
        case Key::Num2:
        case Key::Numpad2:
            m_commands.emplace_back(SelectPlantCommand{PlantType::Wallnut});
            break;
        case Key::Num3:
        case Key::Numpad3:
            m_commands.emplace_back(SelectPlantCommand{PlantType::Sunflower});
            break;
        case Key::R:
            m_commands.emplace_back(RestartCommand{});
            break;
        case Key::Escape:
            if (m_world && isTerminal(m_world->state())) {
                m_window.close();
            } else if (m_world && m_world->state() == GameState::Paused) {
                m_commands.emplace_back(ResumeCommand{});
            } else {
                m_commands.emplace_back(PauseCommand{});
            }
            break;
        default:
            break;
    }
}

void Application::handleMousePressed(const sf::Event::MouseButtonPressed& mouse) {
    if (mouse.button != sf::Mouse::Button::Left || m_screen != Screen::InGame) {
        return;
    }

    // A click on the seed bar selects a plant instead of reaching the lawn.
    const Vec2 screenPoint{static_cast<float>(mouse.position.x),
                           static_cast<float>(mouse.position.y)};
    for (const PlantType type : {PlantType::Peashooter, PlantType::Wallnut, PlantType::Sunflower}) {
        if (m_hud->packetBounds(type).contains(screenPoint)) {
            m_commands.emplace_back(SelectPlantCommand{type});
            return;
        }
    }

    // The position travels with the event in SFML 3. The original queried the
    // mouse's *current* location instead, so a fast movement between click and
    // poll planted on the wrong tile.
    m_commands.emplace_back(ClickCommand{m_view.toWorld(mouse.position)});
}

void Application::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            return;
        }
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            handleKeyPressed(*key);
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            handleMousePressed(*mouse);
        }
    }
}

void Application::render() {
    m_window.clear(sf::Color{18, 22, 18});

    if (m_world) {
        m_gameRenderer->draw(m_window, *m_world);
        m_hud->draw(m_window, *m_world);
    }

    switch (m_screen) {
        case Screen::MainMenu:
            m_screens->drawMainMenu(m_window);
            break;
        case Screen::InGame:
            if (m_world) {
                if (isTerminal(m_world->state())) {
                    m_screens->drawResult(m_window, m_world->state() == GameState::Won);
                } else if (m_world->state() == GameState::Paused) {
                    m_screens->drawPauseOverlay(m_window);
                }
            }
            break;
    }

    m_window.display();
}

int Application::run() {
    float accumulator = 0.0F;
    m_clock.restart();

    while (m_window.isOpen()) {
        // Clamped so a stalled frame cannot trigger an unbounded catch-up.
        const float frameTime = std::min(m_clock.restart().asSeconds(), kMaxFrameTime);

        m_commands.clear();
        processEvents();

        if (m_world) {
            accumulator += frameTime;
            // Fixed timestep: the simulation advances the same way regardless
            // of the display's refresh rate. The original slept inside the
            // event loop, so pacing came from vsync and a 144Hz monitor ran
            // the game 2.4x too fast.
            bool commandsDelivered = false;
            while (accumulator >= kFixedTimeStep) {
                m_world->step(kFixedTimeStep,
                              commandsDelivered ? std::span<const Command>{} : m_commands);
                commandsDelivered = true;
                accumulator -= kFixedTimeStep;
            }
            // Input must not be dropped on a frame too short to contain a step.
            if (!commandsDelivered && !m_commands.empty()) {
                m_world->step(0.0F, m_commands);
            }
        }

        render();
    }

    return 0;
}

} // namespace pvz::app
