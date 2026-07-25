#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace pvz::app {

/// Thrown when an asset cannot be found or read.
///
/// The original discarded every load result and only failed later, deep inside
/// a draw call, as an uncaught std::out_of_range from a map lookup.
class AssetLoadError final : public std::runtime_error {
public:
    AssetLoadError(const std::filesystem::path& path, std::string_view reason);
};

/// Directory containing the running executable.
///
/// Uses the platform API rather than argv[0], which is not reliable.
[[nodiscard]] std::filesystem::path executableDirectory();

/// Locates the assets directory.
///
/// Searched in order:
///   1. $PVZ_ASSET_DIR
///   2. an explicit override (the --assets flag)
///   3. <exe dir>/assets            -- installed and build-tree layouts
///   4. <exe dir>/../share/pvz/assets
///   5. the source tree, in debug builds only
///
/// Throws AssetLoadError listing every path tried, since a first run that
/// cannot find its assets is the most common failure mode.
[[nodiscard]] std::filesystem::path resolveAssetDirectory(
    const std::filesystem::path& explicitOverride = {});

} // namespace pvz::app
