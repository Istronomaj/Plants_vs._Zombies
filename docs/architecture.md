# Architecture

The project is split into two layers so that the game's rules can be tested
without a graphics context.

```
  pvz              executable; a four-line main()
   |
  pvz_app          SFML 3: window, rendering, input translation
   |
  pvz_core         the simulation. No SFML, no I/O beyond reading a config file
   |
  pvz_tests        links pvz_core only
```

`pvz_core` is where every rule lives. `pvz_app` observes it and feeds it input;
it never reaches in to change game state directly.

## Why the split

The interesting behaviour in a game like this is mostly invisible in a
screenshot: whether victory waits for the last zombie to die or fires when it
spawns, whether a plant you cannot afford still consumes the tile, whether a
pea damages one zombie or all of them. Testing those through a window is
impractical, so the simulation has no window.

A CI job greps `src/core` and `include/pvz/core` for `SFML` and fails if it
finds anything, so this cannot regress quietly.

## The simulation

### World

`World` owns the entities, the board, the economy and the wave, and exposes one
entry point:

```cpp
void World::step(float dt, std::span<const Command> commands);
```

Each step applies queued input, advances the spawner, updates every entity,
removes the dead, and re-evaluates the win/loss condition.

Observation is read-only: `entities()` returns a `std::span` of const pointers
and `board()` returns a const reference, so a renderer cannot accidentally
mutate the simulation while drawing it.

### Commands

Input reaches the simulation as a `std::variant`:

```cpp
using Command = std::variant<ClickCommand, SelectPlantCommand,
                             PauseCommand, ResumeCommand, RestartCommand>;
```

`ClickCommand` carries a world position, not an entity or a tile. Deciding what
a click means — collect the sun under it, or plant on the tile beneath it — is a
game rule, so `World` makes that decision and a test can check it.

### Fixed timestep

`World::step` expects `dt == kFixedTimeStep` (1/120 s). The application
accumulates real frame time and consumes it in whole steps:

```cpp
accumulator += std::min(frameTime, kMaxFrameTime);
while (accumulator >= kFixedTimeStep) {
    world.step(kFixedTimeStep, commands);
    accumulator -= kFixedTimeStep;
}
```

Two tests hold this in place: one runs two worlds from the same seed and
compares full state snapshots; the other compares a world stepped uniformly
against one driven through the accumulator with deliberately uneven frame times.

120 Hz is chosen so the fastest entity moves under one pixel per step, which
keeps discrete AABB collision reliable without swept tests.

### Entities

```
Entity                     kind tag, position, hitbox, health
├── Plant                  cost, the tile it occupies, an action timer
│   ├── Peashooter         fires down its lane
│   ├── Sunflower          produces sun
│   └── Wallnut            no action; pure blocker
├── Zombie / BasicZombie   walks, stops to eat, damages the house
├── Projectile             travels right, hits one zombie
└── Sun                    drifts to a point, expires if not collected
```

Type identification is an `EntityKind` enum. `World` re-partitions entities into
per-category and per-lane views once per step, so a peashooter checking for a
target scans only its own lane rather than every entity in the game.

Timers are floats that carry their remainder:

```cpp
m_cooldown -= dt;
while (m_cooldown <= 0.0F) {
    m_cooldown += m_actionInterval;   // += keeps the leftover time
    action(world);
}
```

Assigning the interval instead of adding it would discard the fraction each
cycle, making every rate slightly slower than configured.

### Geometry

`GridGeometry` maps between tiles and world positions using an origin and a cell
size. `cellAt` is the inverse, with an explicit negative guard so a click just
left of the lawn does not truncate onto column 0.

Distances are in *map space* — the coordinate system of `map.png`. The window is
a scaled copy, and `ViewTransform` in the app layer converts between the two, so
the simulation is unaffected by window size.

### Randomness

`Rng` is an interface. `Mt19937Rng` seeds once at construction;
`SequenceRng` replays a scripted list so a test can state exactly which lane the
next zombie uses. `World` takes ownership of one, which is what makes the
determinism test possible.

## The application layer

`Application` owns the window, the resources and the `World`. Renderers take an
`sf::RenderTarget&` per call and own only their resources, which keeps them
usable against an `sf::RenderTexture` if golden-image tests are ever wanted.

- `ResourceManager` keys textures by enum and loads everything eagerly, throwing
  `AssetLoadError` naming the file on failure.
- `AssetPaths` locates the asset directory by searching an ordered candidate
  list, using the platform API rather than `argv[0]` to find the executable.
- `GameRenderer` reuses a sprite and a draw list across frames and sorts by an
  integer layer, so steady-state rendering does not allocate.
- `SpriteTable` maps each `EntityKind` to a texture and a target display height.
  Expressing size as a height rather than a scale factor means replacing a
  sprite with a different-resolution version does not change how large it draws.

`GameState` (playing, paused, won, lost) belongs to the simulation because those
are facts about the match. `Screen` (title, in-game) belongs to the application,
because a title screen is a presentation concern — there is no `World` behind it.

## Testing

`pvz_tests` links `pvz_core` and GoogleTest, and nothing else.

| File | Covers |
| --- | --- |
| `VictoryTest.cpp` | Win and loss conditions, pause, restart |
| `PlantingTest.cpp` | Placement, cost, tile occupancy and release |
| `CombatTest.cpp` | Projectiles, biting, lane targeting, reaching the house |
| `EconomyTest.cpp` | Sun production, collection, expiry, click priority |
| `SimulationTest.cpp` | Determinism, framerate independence, the spawner |
| `GeometryTest.cpp` | Tile mapping, bounds, rectangle and vector maths |
| `ConfigTest.cpp` | Balance defaults and TOML overrides |
| `RngTest.cpp` | Seeding and range behaviour |

`TestHelpers.hpp` provides `makeWorld`, `advance` and `sendCommand`. Its default
configuration keeps a wave pending but pushes the first spawn far into the
future: a match with nothing left to spawn and nothing alive is immediately won,
and a finished world stops stepping, so tests that need the clock to keep
running must avoid that state.

Run the suite without any graphics dependency:

```sh
cmake --preset core-only && cmake --build --preset core-only && ctest --preset core-only
```
