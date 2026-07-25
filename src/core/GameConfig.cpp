#include "pvz/core/GameConfig.hpp"

#include <toml++/toml.hpp>

namespace pvz {
namespace {

/// Reads `table[key]` into `target` if present and of the right type.
/// Absent keys leave the default in place, so an override file only needs to
/// mention what it actually changes.
template <typename T>
void readInto(const toml::table& table, std::string_view key, T& target) {
    if (const auto value = table[key].value<T>()) {
        target = *value;
    }
}

void readPlant(const toml::table& root, std::string_view name, PlantStats& stats) {
    const auto* section = root[name].as_table();
    if (section == nullptr) {
        return;
    }
    readInto(*section, "cost", stats.cost);
    readInto(*section, "health", stats.health);
    readInto(*section, "action_interval", stats.actionInterval);
    readInto(*section, "damage", stats.damage);
    readInto(*section, "projectile_speed", stats.projectileSpeed);
}

} // namespace

const PlantStats& GameConfig::plant(PlantType type) const noexcept {
    switch (type) {
        case PlantType::Peashooter:
            return peashooter;
        case PlantType::Wallnut:
            return wallnut;
        case PlantType::Sunflower:
            return sunflower;
    }
    return peashooter;
}

std::optional<GameConfig> GameConfig::loadFromToml(const std::filesystem::path& path,
                                                   std::string& error) {
    error.clear();

    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        error = "balance file not found: " + path.string();
        return std::nullopt;
    }

    toml::table root;
    try {
        root = toml::parse_file(path.string());
    } catch (const toml::parse_error& e) {
        error = "failed to parse " + path.string() + ": " + std::string{e.description()};
        return std::nullopt;
    }

    GameConfig config{};

    if (const auto* economy = root["economy"].as_table()) {
        readInto(*economy, "starting_sun", config.startingSun);
        readInto(*economy, "sun_value", config.sunValue);
        readInto(*economy, "player_health", config.playerHealth);
        readInto(*economy, "sun_lifetime", config.sunLifetime);
        readInto(*economy, "sun_drift_speed", config.sunDriftSpeed);
        readInto(*economy, "sun_scatter_radius", config.sunScatterRadius);
    }

    if (const auto* waves = root["waves"].as_table()) {
        readInto(*waves, "zombie_count_min", config.zombieCountMin);
        readInto(*waves, "zombie_count_max", config.zombieCountMax);
        readInto(*waves, "spawn_interval_min", config.spawnIntervalMin);
        readInto(*waves, "spawn_interval_max", config.spawnIntervalMax);
        readInto(*waves, "first_spawn_delay", config.firstSpawnDelay);
    }

    readPlant(root, "peashooter", config.peashooter);
    readPlant(root, "wallnut", config.wallnut);
    readPlant(root, "sunflower", config.sunflower);

    if (const auto* zombie = root["basic_zombie"].as_table()) {
        readInto(*zombie, "health", config.basicZombie.health);
        readInto(*zombie, "speed", config.basicZombie.speed);
        readInto(*zombie, "damage", config.basicZombie.damage);
        readInto(*zombie, "bite_interval", config.basicZombie.biteInterval);
    }

    return config;
}

} // namespace pvz
