# Contributing

## Building and testing

```sh
cmake --preset core-only        # core + tests, no SFML download
cmake --build --preset core-only
ctest --preset core-only
```

Use `dev-debug` or `dev-release` when you need the game itself. `core-only` is
the quickest loop for anything that does not touch rendering, and it works on a
machine with no graphics libraries installed.

Before opening a pull request:

```sh
cmake --preset dev-debug        # warnings as errors
cmake --build --preset dev-debug
ctest --preset dev-debug
```

## The one structural rule

**`pvz_core` must not depend on SFML.** Nothing under `src/core` or
`include/pvz/core` may include an SFML header or use an `sf::` type. That
constraint is why the simulation can be tested at all, and CI enforces it.

Anything involving a window, a texture, an event or a pixel belongs in
`src/app`. If a rule seems to need one of those, it usually needs a plain value
instead — see how `ClickCommand` carries a position rather than an SFML event.

## Style

`.clang-format` and `.editorconfig` are in the repository; please run
clang-format on anything you touch. clang-tidy runs in CI and is blocking, so
check locally if you are changing much:

```sh
cmake --preset ci-linux-clang
find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy -p build/ci-linux-clang
```

Conventions worth knowing:

- Private and protected members are prefixed `m_`.
- Getters are `[[nodiscard]]` and named for the thing, not `getThing()`.
- Comments explain *why*, not *what*. Prefer none to a restatement of the code.

## Tests

New behaviour needs a test. Bug fixes especially — the test should fail against
the old behaviour, so write it that way round where you can.

Tests use `TestHelpers.hpp`. Note that `quietConfig()` deliberately keeps a
zombie pending: a match with nothing left to spawn and nothing alive is won
immediately, and a finished `World` stops advancing, which will silently
invalidate anything measuring elapsed time.

## Balance changes

Gameplay values belong in `assets/balance.toml` and the `GameConfig` defaults,
not in code. If you find a number in a `.cpp` file that a player would notice,
it probably wants to move.
