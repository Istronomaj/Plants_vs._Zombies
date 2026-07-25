cmake_minimum_required(VERSION 3.24)  # FetchContent FIND_PACKAGE_ARGS

project(PlantsVsZombies
        VERSION 1.0.0
        DESCRIPTION "A Plants vs. Zombies remake with a headless, unit-tested game core"
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GNUInstallDirs)
include(FetchContent)
include(CompilerWarnings)
include(Sanitizers)

option(PVZ_BUILD_TESTS        "Build unit tests"                ON)
option(PVZ_BUILD_APP          "Build the graphical application" ON)
option(PVZ_WARNINGS_AS_ERRORS "Treat warnings as errors"        OFF)
option(PVZ_ENABLE_SANITIZERS  "Enable ASan + UBSan"             OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# toml++ is header-only and has no graphics dependency, so the core library can
# use it without breaking the "core must not know about SFML" invariant.
FetchContent_Declare(tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG        v3.4.0
        GIT_SHALLOW    TRUE
        FIND_PACKAGE_ARGS)
FetchContent_MakeAvailable(tomlplusplus)

add_subdirectory(src/core)

if(PVZ_BUILD_APP)
    # The game has no audio or networking; disabling them removes the OpenAL /
    # vorbis / FLAC dependency chain and a large chunk of the from-source build.
    set(SFML_BUILD_AUDIO    OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK  OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC      OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(SFML
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG        3.0.2
            GIT_SHALLOW    TRUE
            FIND_PACKAGE_ARGS 3 COMPONENTS Graphics Window System)
    FetchContent_MakeAvailable(SFML)

    add_subdirectory(src/app)

    add_executable(pvz src/main.cpp)
    target_link_libraries(pvz PRIVATE pvz_app)
    pvz_set_warnings(pvz)
    pvz_enable_sanitizers(pvz)

    # Assets must sit next to the binary. The previous build had every copy
    # step commented out, so installed and released binaries could not start.
    add_custom_command(TARGET pvz POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:pvz>/assets"
            COMMENT "Copying assets next to the executable")

    install(TARGETS pvz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_BINDIR}/assets)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(PVZ_BUILD_TESTS)
    enable_testing()

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # avoids MSVC CRT mismatch
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)  # keeps gtest out of our archives
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.15.2
            GIT_SHALLOW    TRUE
            FIND_PACKAGE_ARGS NAMES GTest)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()
