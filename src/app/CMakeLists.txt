# pvz_app: the SFML presentation layer.
#
# This is the only target that knows SFML exists. pvz_core does not link it at
# all, which is what keeps the simulation testable without a window; a CI job
# asserts that no SFML reference appears under src/core or include/pvz/core.
#
# SFML is PUBLIC here because this layer's own headers expose sf:: types in
# their interfaces (sf::RenderTarget&, sf::Text members, and so on).
add_library(pvz_app STATIC
        Application.cpp
        AssetPaths.cpp
        GameRenderer.cpp
        HudRenderer.cpp
        ResourceManager.cpp
        ScreenRenderer.cpp
)

target_include_directories(pvz_app PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
)

target_link_libraries(pvz_app
        PUBLIC pvz_core SFML::Graphics SFML::Window SFML::System
)

# Lets a debug build started from an IDE find the assets regardless of its
# working directory. Release builds rely on the copy next to the binary.
target_compile_definitions(pvz_app PRIVATE
        "$<$<CONFIG:Debug>:PVZ_SOURCE_ASSET_DIR=\"${CMAKE_SOURCE_DIR}/assets\">"
)

pvz_set_warnings(pvz_app)
pvz_enable_sanitizers(pvz_app)
