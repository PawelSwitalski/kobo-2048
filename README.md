# Kobo 2048

A native recreation of [gabrielecirulli/2048](https://github.com/gabrielecirulli/2048)
for Kobo e-readers: swipe to slide and merge tiles on a 4×4 grid, with a
persisted best score, win/game-over detection, full resume of an
in-progress game, and a Color/Black & White display theme with a
tunable e-ink full-refresh cadence. Fully offline, one armhf binary for
the whole Kobo lineup.

Built from the kobo-games-template (see [SETUP.md](SETUP.md) for the
template this project started from, if you want to build your own game the
same way).

## What's included

- 4-layer architecture: `core` (pure logic — board, game session, best
  score, display settings) / `persist` (atomic JSON save/load) / `platform`
  (Renderer + TouchInput abstraction, `kobo/` and `sdl/` backends) / `ui`
  (renderer-agnostic screens and widgets)
- The game itself: swipe-to-move core loop, win/game-over notifications,
  New Game, and a Settings screen (color mode, full-refresh cadence)
- Desktop simulator (SDL2) and Kobo device (FBInk, cross-compiled) build
  flavors from one CMake project
- NickelMenu + KFMon device packaging (`tools/package.sh`)
- CI: host tests → Kobo cross-build → tag-triggered GitHub release
- Spec-driven-development tooling (spec-kit) under `.specify/` and
  `.claude/skills/` — see `specs/001-2048-game/` for this feature's
  spec/plan/tasks
- `tools/rename-project.sh` to rebrand a fresh copy into a new project

## Documentation

| Document | Contents |
|---|---|
| [SETUP.md](SETUP.md) | Start a new project from this template |
| [Installation](docs/installation.md) | Install, launchers (NickelMenu / KFMon), uninstall |
| [Settings](docs/settings.md) | Launcher options, touch calibration, device files |
| [Building](docs/building.md) | Host tests, desktop simulator, Kobo cross-build, CI and releases |
| [Platform abstraction contract](docs/contracts/platform-abstraction.md) | The Renderer/TouchInput interfaces `ui` and backends must honor |
| [Install layout contract](docs/contracts/install-layout.md) | Device package shape and launcher integration |

## Building in short

```bash
# unit tests + desktop simulator (Windows/Linux/macOS, CMake + C++17)
cmake -B build/sim -DKOBO_2048_BACKEND=sdl && cmake --build build/sim --config Release

# device binary (Linux/WSL2 + koxtoolchain)
tools/build-fbink.sh
cmake -B build/kobo -DCMAKE_TOOLCHAIN_FILE=cmake/kobo-toolchain.cmake -DKOBO_2048_BACKEND=fbink
cmake --build build/kobo && tools/package.sh build/kobo
```

Details in [docs/building.md](docs/building.md).

## License

[MIT](LICENSE). Vendored third-party components keep their own licenses:
FBInk, nlohmann/json, doctest, stb_truetype, SDL2 (simulator only) and the
DejaVu Sans fonts (`dist/.adds/kobo_2048/assets/FONT-LICENSE.txt`).
