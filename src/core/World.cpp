#include "pvz/core/World.hpp"

#include "pvz/core/EntityFactory.hpp"
#include "pvz/core/Plant.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace pvz {

World::World(GameConfig config, std::unique_ptr<Rng> rng)
    : m_config(std::move(config)),
      m_geometry(m_config.geometry()),
      m_rng(std::move(rng)),
      m_board(m_config.rows, m_config.cols),
      m_zombiesByLane(static_cast<std::size_t>(m_config.rows)) {
    startMatch();
}

void World::startMatch() {
    m_entities.clear();
    m_pendingSpawns.clear();
    m_plantsView.clear();
    for (auto& lane : m_zombiesByLane) {
        lane.clear();
    }
    m_board.clear();

    m_state = GameState::Playing;
    m_sun = m_config.startingSun;
    m_playerHealth = m_config.playerHealth;
    m_selectedPlant = PlantType::Peashooter;
    m_zombiesPending = m_rng->nextInt(m_config.zombieCountMin, m_config.zombieCountMax);
    m_zombiesAlive = 0;
    m_spawnTimer = m_config.firstSpawnDelay;
    m_elapsed = 0.0F;
}

void World::reset() {
    startMatch();
}

int World::laneOf(float y) const noexcept {
    const float raw = (y - m_geometry.origin().y) / m_geometry.cellSize().y + 0.5F;
    const int lane = static_cast<int>(std::floor(raw));
    return std::clamp(lane, 0, m_geometry.rows() - 1);
}

std::span<Entity* const> World::zombiesInLane(int row) const noexcept {
    if (row < 0 || static_cast<std::size_t>(row) >= m_zombiesByLane.size()) {
        return {};
    }
    return m_zombiesByLane[static_cast<std::size_t>(row)];
}

void World::spawn(std::unique_ptr<Entity> entity) {
    // Buffered rather than appended directly: entities spawn from inside the
    // update loop, and appending there would invalidate the iteration.
    m_pendingSpawns.push_back(std::move(entity));
}

void World::placeNow(std::unique_ptr<Entity> entity) {
    if (entity == nullptr) {
        return;
    }
    if (entity->category() == EntityCategory::Zombie) {
        ++m_zombiesAlive;
    } else if (entity->category() == EntityCategory::Plant) {
        if (const auto* plant = dynamic_cast<const Plant*>(entity.get())) {
            m_board.occupy(plant->cell());
        }
    }
    m_entities.push_back(std::move(entity));
    rebuildViews();
}

void World::setZombiesPending(int count) noexcept {
    m_zombiesPending = std::max(0, count);
}

void World::damagePlayer(int amount) noexcept {
    m_playerHealth = std::max(0, m_playerHealth - amount);
}

void World::addSun(int amount) noexcept {
    m_sun += amount;
}

void World::step(float dt, std::span<const Command> commands) {
    for (const Command& command : commands) {
        applyCommand(command);
    }

    if (m_state != GameState::Playing) {
        return;
    }

    m_elapsed += dt;

    rebuildViews();
    updateSpawner(dt);

    // Index-based: entities may spawn during iteration, and the buffer above
    // means the vector itself is not resized here.
    for (std::size_t i = 0; i < m_entities.size(); ++i) {
        if (m_entities[i]->isAlive()) {
            m_entities[i]->update(*this, dt);
        }
    }

    resolveDeaths();

    m_entities.insert(m_entities.end(), std::make_move_iterator(m_pendingSpawns.begin()),
                      std::make_move_iterator(m_pendingSpawns.end()));
    m_pendingSpawns.clear();

    checkVictory();
}

void World::applyCommand(const Command& command) {
    std::visit(
        [this](const auto& cmd) {
            using T = std::decay_t<decltype(cmd)>;

            if constexpr (std::is_same_v<T, ClickCommand>) {
                if (m_state == GameState::Playing) {
                    handleClick(cmd.worldPosition);
                }
            } else if constexpr (std::is_same_v<T, SelectPlantCommand>) {
                if (m_state == GameState::Playing) {
                    m_selectedPlant = cmd.type;
                }
            } else if constexpr (std::is_same_v<T, PauseCommand>) {
                if (m_state == GameState::Playing) {
                    m_state = GameState::Paused;
                }
            } else if constexpr (std::is_same_v<T, ResumeCommand>) {
                if (m_state == GameState::Paused) {
                    m_state = GameState::Playing;
                }
            } else if constexpr (std::is_same_v<T, RestartCommand>) {
                startMatch();
            }
        },
        command);
}

void World::handleClick(Vec2 worldPosition) {
    // Sun takes priority over planting, so a sun drifting over an empty tile
    // can be collected without also spending it on that tile.
    if (tryCollectSun(worldPosition)) {
        return;
    }
    if (const auto cell = m_geometry.cellAt(worldPosition)) {
        tryPlant(*cell, m_selectedPlant);
    }
}

bool World::tryCollectSun(Vec2 worldPosition) {
    for (auto& entity : m_entities) {
        if (entity->kind() != EntityKind::Sun || !entity->isAlive()) {
            continue;
        }
        if (entity->bounds().contains(worldPosition)) {
            entity->kill();
            addSun(m_config.sunValue);
            return true;
        }
    }
    return false;
}

bool World::tryPlant(GridCell cell, PlantType type) {
    if (!m_board.inBounds(cell) || m_board.isOccupied(cell)) {
        return false;
    }

    const int cost = m_config.plant(type).cost;
    if (m_sun < cost) {
        // Nothing is mutated on this path. The original charged the cost in one
        // function and marked the tile occupied in the caller, so a click made
        // without enough sun left the tile permanently unusable.
        return false;
    }

    m_sun -= cost;
    m_board.occupy(cell);
    m_pendingSpawns.push_back(
        EntityFactory::createPlant(type, m_geometry.cellCenter(cell), cell, m_config));
    return true;
}

void World::updateSpawner(float dt) {
    if (m_zombiesPending <= 0) {
        return;
    }

    m_spawnTimer -= dt;
    // `<=` and a loop, not `== 0`: an equality test misses the moment entirely
    // if the timer steps past zero, and never fires again.
    while (m_spawnTimer <= 0.0F && m_zombiesPending > 0) {
        spawnZombie();
        m_spawnTimer +=
            m_rng->nextFloat(m_config.spawnIntervalMin, m_config.spawnIntervalMax);
    }
}

void World::spawnZombie() {
    const int lane = m_rng->nextInt(0, m_geometry.rows() - 1);
    const Vec2 position{m_geometry.spawnX(), m_geometry.laneY(lane)};

    m_pendingSpawns.push_back(EntityFactory::createBasicZombie(position, m_config));
    --m_zombiesPending;
    ++m_zombiesAlive;
}

void World::rebuildViews() {
    m_plantsView.clear();
    for (auto& lane : m_zombiesByLane) {
        lane.clear();
    }

    for (auto& entity : m_entities) {
        if (!entity->isAlive()) {
            continue;
        }
        switch (entity->category()) {
            case EntityCategory::Plant:
                m_plantsView.push_back(entity.get());
                break;
            case EntityCategory::Zombie: {
                const int lane = laneOf(entity->position().y);
                m_zombiesByLane[static_cast<std::size_t>(lane)].push_back(entity.get());
                break;
            }
            case EntityCategory::Projectile:
            case EntityCategory::Pickup:
                break;
        }
    }
}

void World::resolveDeaths() {
    for (auto& entity : m_entities) {
        if (entity->isAlive()) {
            continue;
        }
        if (entity->category() == EntityCategory::Zombie) {
            m_zombiesAlive = std::max(0, m_zombiesAlive - 1);
        } else if (entity->category() == EntityCategory::Plant) {
            // Uses the cell the plant remembers, rather than matching its
            // position against the tile table by integer equality.
            if (const auto* plant = dynamic_cast<const Plant*>(entity.get())) {
                m_board.free(plant->cell());
            }
        }
    }

    std::erase_if(m_entities, [](const std::unique_ptr<Entity>& e) { return !e->isAlive(); });

    // Views hold raw pointers into the vector just erased from.
    m_plantsView.clear();
    for (auto& lane : m_zombiesByLane) {
        lane.clear();
    }
}

void World::checkVictory() noexcept {
    // Loss is checked first, so a zombie reaching the house on the same step as
    // the final kill counts as a loss rather than an ambiguous win.
    if (m_playerHealth <= 0) {
        m_state = GameState::Lost;
        return;
    }
    if (m_zombiesPending == 0 && m_zombiesAlive == 0) {
        m_state = GameState::Won;
    }
}

} // namespace pvz
