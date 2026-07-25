include(GoogleTest)

# Links pvz_core only -- no SFML, no window. If this ever needs a graphics
# dependency, the core/app layering has been broken.
add_executable(pvz_tests
        CombatTest.cpp
        ConfigTest.cpp
        EconomyTest.cpp
        GeometryTest.cpp
        PlantingTest.cpp
        RngTest.cpp
        SimulationTest.cpp
        VictoryTest.cpp
)

target_link_libraries(pvz_tests PRIVATE pvz_core GTest::gtest_main)

target_compile_definitions(pvz_tests PRIVATE
        PVZ_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

pvz_set_warnings(pvz_tests)
pvz_enable_sanitizers(pvz_tests)

gtest_discover_tests(pvz_tests DISCOVERY_TIMEOUT 30)
