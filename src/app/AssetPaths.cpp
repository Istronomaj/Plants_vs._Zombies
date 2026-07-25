#include "pvz/app/AssetPaths.hpp"

#include <cstdlib>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <vector>
#else
#include <unistd.h>
#endif

namespace pvz::app {
namespace {

/// A file that must exist for a directory to count as the asset root.
constexpr const char* kSentinelAsset = "map.png";

[[nodiscard]] bool looksLikeAssetDir(const std::filesystem::path& dir) {
    std::error_code ec;
    return std::filesystem::exists(dir / kSentinelAsset, ec);
}

[[nodiscard]] std::filesystem::path fromEnvironment() {
#if defined(_MSC_VER)
    char* buffer = nullptr;
    std::size_t size = 0;
    if (_dupenv_s(&buffer, &size, "PVZ_ASSET_DIR") == 0 && buffer != nullptr) {
        std::filesystem::path path{buffer};
        std::free(buffer);
        return path;
    }
    return {};
#else
    if (const char* value = std::getenv("PVZ_ASSET_DIR")) {
        return std::filesystem::path{value};
    }
    return {};
#endif
}

} // namespace

AssetLoadError::AssetLoadError(const std::filesystem::path& path, std::string_view reason)
    : std::runtime_error(path.string() + ": " + std::string{reason}) {}

std::filesystem::path executableDirectory() {
#if defined(_WIN32)
    std::wstring buffer(MAX_PATH, L'\0');
    for (;;) {
        const DWORD written =
            GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (written == 0) {
            return std::filesystem::current_path();
        }
        if (written < buffer.size()) {
            buffer.resize(written);
            break;
        }
        buffer.resize(buffer.size() * 2);
    }
    return std::filesystem::path{buffer}.parent_path();

#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size + 1, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return std::filesystem::current_path();
    }
    std::error_code ec;
    const auto canonical = std::filesystem::canonical(buffer.data(), ec);
    return ec ? std::filesystem::current_path() : canonical.parent_path();

#else
    std::error_code ec;
    const auto self = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) {
        return std::filesystem::current_path();
    }
    return self.parent_path();
#endif
}

std::filesystem::path resolveAssetDirectory(const std::filesystem::path& explicitOverride) {
    const std::filesystem::path exeDir = executableDirectory();

    std::vector<std::filesystem::path> candidates;
    if (const auto fromEnv = fromEnvironment(); !fromEnv.empty()) {
        candidates.push_back(fromEnv);
    }
    if (!explicitOverride.empty()) {
        candidates.push_back(explicitOverride);
    }
    candidates.push_back(exeDir / "assets");
    candidates.push_back(exeDir / ".." / "share" / "pvz" / "assets");
#ifdef PVZ_SOURCE_ASSET_DIR
    // Lets the game run from an IDE with an arbitrary working directory.
    candidates.emplace_back(PVZ_SOURCE_ASSET_DIR);
#endif
    candidates.push_back(std::filesystem::current_path() / "assets");

    for (const auto& candidate : candidates) {
        if (looksLikeAssetDir(candidate)) {
            std::error_code ec;
            auto canonical = std::filesystem::canonical(candidate, ec);
            return ec ? candidate : canonical;
        }
    }

    std::ostringstream tried;
    tried << "could not locate the assets directory. Tried:";
    for (const auto& candidate : candidates) {
        tried << "\n  " << candidate.string();
    }
    tried << "\nSet PVZ_ASSET_DIR or pass --assets <path>.";
    throw AssetLoadError{kSentinelAsset, tried.str()};
}

} // namespace pvz::app
