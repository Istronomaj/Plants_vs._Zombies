# Plants vs. Zombies

[![CI](https://github.com/stefanpeiculeasa/PlantsVsZombies-CPP/actions/workflows/ci.yml/badge.svg)](https://github.com/stefanpeiculeasa/PlantsVsZombies-CPP/actions/workflows/ci.yml)
[![Sanitizers](https://github.com/stefanpeiculeasa/PlantsVsZombies-CPP/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/stefanpeiculeasa/PlantsVsZombies-CPP/actions/workflows/sanitizers.yml)
[![Static analysis](https://github.com/stefanpeiculeasa/PlantsVsZombies-CPP/actions/workflows/analysis.yml/badge.svg)](https://github.com/stefanpeiculeasa/PlantsVsZombies-CPP/actions/workflows/analysis.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A lane-defence game in the style of *Plants vs. Zombies*, built on a **headless
simulation core** that runs and is fully unit-tested without a graphics context.

## Features

- **Graphics-free game core.** All rules — movement, combat, the sun economy,
  wave spawning, win and loss — live in `pvz_core`, which does not link SFML.
  The entire game can be driven from a test with no window. A CI job fails the
  build if a graphics dependency ever creeps into that layer.
- **Deterministic, framerate-independent simulation.** A fixed 120 Hz timestep
  behind an accumulator, and a seedable RNG. The same seed produces the same
  match, and the outcome does not change with the display's refresh rate.
- **Data-driven balance.** Every tunable value lives in [`assets/balance.toml`](assets/balance.toml).
  Retune the game without recompiling; delete the file and the built-in
  defaults take over.
- **64 unit tests**, run on Linux (GCC and Clang), macOS and Windows, plus a
  separate ASan/UBSan job.

## Controls

| Input | Action |
| --- | --- |
| <kbd>Enter</kbd> | Start the game from the title screen |
| <kbd>1</kbd> / <kbd>2</kbd> / <kbd>3</kbd> | Select peashooter / wall-nut / sunflower |
| Left click | Place the selected plant, or collect sun |
| Click a seed packet | Select that plant |
| <kbd>Esc</kbd> | Pause, resume, or quit from the result screen |
| <kbd>R</kbd> | Restart |

Clicking a sun always collects it, even when it is floating over an empty tile,
so collecting never accidentally spends your sun on a plant.

| Plant | Cost | Role |
| --- | --- | --- |
| Peashooter | 4 | Fires down its lane at any zombie ahead of it |
| Wall-nut | 5 | Absorbs damage; has no attack |
| Sunflower | 5 | Produces sun every 10 seconds |

**Win** by defeating the whole wave. **Lose** if zombies reduce your health to
zero.

## Building

Requires **CMake 3.24+** and a **C++20** compiler. SFML 3, toml++ and GoogleTest
are fetched automatically; a system SFML 3 is used instead if one is installed.

On Debian/Ubuntu, install SFML's dependencies first:

```sh
sudo apt-get install -y ninja-build libxrandr-dev libxcursor-dev libxi-dev \
                        libudev-dev libfreetype-dev libgl1-mesa-dev libegl1-mesa-dev
```

Then, on any platform:

```sh
cmake --preset dev-release
cmake --build --preset dev-release
./build/dev-release/pvz
```

<details>
<summary><b>Windows: "cmake: command not found"</b></summary>

If you build with Visual Studio Build Tools, its CMake and Ninja are not on the
default `PATH`, and the compiler needs the environment `VsDevCmd.bat` sets up.
Either use a **Developer Command Prompt**, or prefix commands with the wrapper:

```sh
./scripts/devshell.sh cmake --preset dev-release
./scripts/devshell.sh cmake --build --preset dev-release
./scripts/devshell.sh ctest --preset dev-release
```

It locates the toolchain with `vswhere` and runs the command inside it. On
Linux and macOS it just runs the command unchanged, so it is safe either way.

</details>

| Preset | Purpose |
| --- | --- |
| `dev-release` | Optimised build |
| `dev-debug` | Debug build, warnings as errors |
| `core-only` | Core plus tests, skips SFML entirely — fastest way to run the suite |
| `asan` | Tests under AddressSanitizer and UBSan |

### Running the tests

```sh
cmake --preset core-only
cmake --build --preset core-only
ctest --preset core-only
```

`core-only` needs no graphics libraries and does not download SFML, so it works
on a headless machine.

## Command-line options

```
pvz [--assets <dir>] [--balance <file>] [--scale <factor>] [--seed <number>]
```

| Option | Meaning |
| --- | --- |
| `--assets <dir>` | Where to find the game's assets |
| `--balance <file>` | A TOML file overriding the balance values |
| `--scale <factor>` | Window size as a multiple of the map (0.5–4.0, default 2.0) |
| `--seed <number>` | Fixed RNG seed, for a reproducible match |

Assets are found automatically: `$PVZ_ASSET_DIR`, then `--assets`, then next to
the executable, then the Unix `share/pvz/assets` path. If none of them work the
error lists every location tried.

## Architecture

```
          pvz (executable)
                 |
          pvz_app  ── SFML 3 ── window, sprites, HUD, input
                 |
          pvz_core ───────────── simulation: no graphics dependency
                 |
          pvz_tests ──────────── 64 tests, no window required
```

`pvz_app` translates SFML events into plain `Command` values and hands them to
`World::step`, which is the only entry point into the simulation. Because
`pvz_core` knows nothing about SFML, the tests construct a `World` directly and
drive it with those same commands.

The split is what makes the interesting cases testable at all — that victory
waits for the last zombie to *die* rather than to spawn, that a plant you cannot
afford does not consume the tile, that a pea damages one zombie rather than
every zombie it overlaps.

See [docs/architecture.md](docs/architecture.md) for detail.

## Balance configuration

`assets/balance.toml` is loaded at startup if present. Every key is optional:

```toml
[economy]
starting_sun = 20
player_health = 100

[waves]
zombie_count_min = 15
zombie_count_max = 30
first_spawn_delay = 5.0

[peashooter]
cost = 4
damage = 30
action_interval = 2.0
```

A missing or malformed file is not fatal — the game logs the problem and uses
its defaults.

## License

Source code is released under the [MIT License](LICENSE).

The sprites and font under `assets/` are used for educational, non-commercial
purposes. Several are derived from the commercial game *Plants vs. Zombies*
(© Electronic Arts / PopCap Games) and are **not** covered by that license.
