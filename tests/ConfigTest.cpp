#include "pvz/core/GameConfig.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace pvz {
namespace {

/// Writes a temporary TOML file and removes it on destruction.
class TempToml {
public:
    explicit TempToml(std::string_view contents) {
        m_path = std::filesystem::temp_directory_path() /
                 ("pvz_balance_test_" + std::to_string(s_counter++) + ".toml");
        std::ofstream out(m_path);
        out << contents;
    }

    TempToml(const TempToml&) = delete;
    TempToml& operator=(const TempToml&) = delete;
    TempToml(TempToml&&) = delete;
    TempToml& operator=(TempToml&&) = delete;

    ~TempToml() {
        std::error_code ec;
        std::filesystem::remove(m_path, ec);
    }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
    static inline int s_counter = 0;
};

TEST(Config, DefaultsArePlayable) {
    const GameConfig config{};
    EXPECT_GT(config.startingSun, 0);
    EXPECT_GT(config.playerHealth, 0);
    EXPECT_GE(config.zombieCountMax, config.zombieCountMin);
    EXPECT_GE(config.spawnIntervalMax, config.spawnIntervalMin);
    EXPECT_GT(config.rows, 0);
    EXPECT_GT(config.cols, 0);

    // The player must be able to afford at least one plant immediately,
    // otherwise the opening move is impossible.
    EXPECT_GE(config.startingSun, config.sunflower.cost);
}

TEST(Config, PlantLookupReturnsMatchingStats) {
    const GameConfig config{};
    EXPECT_EQ(config.plant(PlantType::Peashooter).cost, config.peashooter.cost);
    EXPECT_EQ(config.plant(PlantType::Wallnut).health, config.wallnut.health);
    EXPECT_EQ(config.plant(PlantType::Sunflower).actionInterval, config.sunflower.actionInterval);
}

/// Wallnut is a pure blocker: it has no periodic action.
TEST(Config, WallnutHasNoAction) {
    const GameConfig config{};
    EXPECT_FLOAT_EQ(config.wallnut.actionInterval, 0.0F);
    EXPECT_EQ(config.wallnut.damage, 0);
}

TEST(Config, MissingFileReportsErrorAndReturnsNullopt) {
    std::string error;
    const auto loaded = GameConfig::loadFromToml("definitely/not/here.toml", error);
    EXPECT_FALSE(loaded.has_value());
    EXPECT_FALSE(error.empty());
}

/// A broken balance file must never stop the game from starting; the caller
/// falls back to defaults, so loadFromToml only has to report the failure.
TEST(Config, MalformedTomlReportsErrorAndDoesNotThrow) {
    const TempToml file{"this is = = not valid toml [[["};
    std::string error;

    std::optional<GameConfig> loaded;
    EXPECT_NO_THROW(loaded = GameConfig::loadFromToml(file.path(), error));
    EXPECT_FALSE(loaded.has_value());
    EXPECT_FALSE(error.empty());
}

TEST(Config, ValidTomlOverridesOnlyListedKeys) {
    const TempToml file{R"(
[economy]
starting_sun = 500
player_health = 42

[peashooter]
cost = 99
)"};

    std::string error;
    const auto loaded = GameConfig::loadFromToml(file.path(), error);

    ASSERT_TRUE(loaded.has_value()) << error;
    EXPECT_EQ(loaded->startingSun, 500);
    EXPECT_EQ(loaded->playerHealth, 42);
    EXPECT_EQ(loaded->peashooter.cost, 99);

    // Untouched keys keep their defaults.
    const GameConfig defaults{};
    EXPECT_EQ(loaded->sunValue, defaults.sunValue);
    EXPECT_EQ(loaded->peashooter.health, defaults.peashooter.health);
    EXPECT_EQ(loaded->basicZombie.health, defaults.basicZombie.health);
}

TEST(Config, EmptyTomlYieldsDefaults) {
    const TempToml file{""};
    std::string error;
    const auto loaded = GameConfig::loadFromToml(file.path(), error);

    ASSERT_TRUE(loaded.has_value()) << error;
    const GameConfig defaults{};
    EXPECT_EQ(loaded->startingSun, defaults.startingSun);
    EXPECT_EQ(loaded->basicZombie.speed, defaults.basicZombie.speed);
}

} // namespace
} // namespace pvz
