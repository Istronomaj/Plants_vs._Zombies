# pvz_core: the headless simulation.
#
# This target must never link SFML. Everything here is plain C++20 so the whole
# game can be driven from unit tests with no window and no graphics context.
add_library(pvz_core STATIC
        Board.cpp
        Entity.cpp
        EntityFactory.cpp
        GameConfig.cpp
        Plant.cpp
        Plants.cpp
        Projectile.cpp
        Rng.cpp
        Sun.cpp
        World.cpp
        Zombie.cpp
)

target_include_directories(pvz_core PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_features(pvz_core PUBLIC cxx_std_20)

target_link_libraries(pvz_core PUBLIC tomlplusplus::tomlplusplus)

pvz_set_warnings(pvz_core)
pvz_enable_sanitizers(pvz_core)
