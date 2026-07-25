# Setup

## What this is

A CMake/C++17 template for Kobo e-reader apps with an FBInk device
rendering backend, an SDL2 desktop simulator, and NickelMenu/KFMon
packaging. This repository is itself a worked example of a real project
built from the template: a full 2048 game (see [README.md](README.md) and
`specs/001-2048-game/` for the spec/plan/tasks it was built from).

## Start a new project

```bash
cp -r kobo-games-template my-new-game && cd my-new-game
tools/rename-project.sh my_new_game --title "My New Game"
git init && git add -A && git commit -m "Initial commit from kobo-games-template"
```

`tools/rename-project.sh` rebrands the CMake project/target names, C++
namespace, device install paths, env var prefixes, and doc/README text from
the `kobo_2048` placeholder to your chosen name. Run `--dry-run` first if you
want to see what it would touch. See the script's header comment for full
details.

## Verify it builds

```bash
# host unit tests
cmake -B build/host -DMY_NEW_GAME_BACKEND=none -DBUILD_TESTS=ON
cmake --build build/host --config Release
ctest --test-dir build/host -C Release --output-on-failure

# desktop simulator
cmake -B build/sim -DMY_NEW_GAME_BACKEND=sdl
cmake --build build/sim --config Release
build/sim/Release/my_new_game --width 1264 --height 1680 --dpi 300
```

(Replace `MY_NEW_GAME`/`my_new_game` with whatever `tools/rename-project.sh`
derived from the name you picked — it prints the exact commands to use at
the end of its run.)

A freshly renamed copy starts from the template's placeholder demo (a
tap-to-increment counter with autosave and a second screen) — swap it for
your own game following the pattern below. This repository has already done
that swap: its `src/core/board.h`, `game_session.h`, etc. and
`src/ui/screens/game_screen.*`/`settings_screen.*` are the finished result,
useful as a worked example.

## Where to add your own logic

- Domain logic goes in `src/core/` — delete the placeholder demo's
  `counter.h`/`counter.cpp` once you have real state to replace it with
  (see `src/core/board.{h,cpp}` / `game_session.{h,cpp}` for what that
  looks like once done).
- Delete `src/ui/screens/counter_screen.*` and `about_screen.*`, add your
  own screens implementing `src/ui/screens/screen.h` (see
  `game_screen.{h,cpp}` / `settings_screen.{h,cpp}`).
- Extend `src/ui/app.h`'s `App` interface with your own state
  accessors/persist hooks (following the shape this project's
  `session()`/`bestScore()`/`settings()` accessors and `move()`/`newGame()`
  mutators show), and update `AppImpl` in `src/main.cpp` to match.
- Persisted state goes through `src/persist/` (`paths.h` resolves where to
  store files; `store.h` does atomic load/save).
- Reusable UI pieces (`Button`, `Label`, `Dialog`) live in `src/ui/widgets.h`.

## Spec-driven workflow

This template carries over the [spec-kit](https://github.com/github/spec-kit)
SDD tooling used to build the project it was extracted from:
`.specify/memory/constitution.md` plus the `.claude/skills/speckit-*`
Claude Code skills (`/speckit-specify`, `/speckit-plan`, `/speckit-tasks`,
`/speckit-implement`, etc.).

Before running `/speckit-specify` for your first real feature, review and
adjust `.specify/memory/constitution.md` — its six principles are already
generic to "a Kobo device app," but you may want to tighten Principle III's
language for your actual domain.

## Device packaging / cross-build

The FBInk cross-compile (`tools/build-fbink.sh` +
`-DKOBO_2048_BACKEND=fbink` + `cmake/kobo-toolchain.cmake`, or the renamed
equivalent after `rename-project.sh`) requires Linux/WSL2 and the
[koxtoolchain](https://github.com/koreader/koxtoolchain) ARM cross-compiler
— it cannot be built or verified on a Windows-only machine. See
[docs/building.md](docs/building.md) for the full steps, or push to GitHub
and let `.github/workflows/build.yml`'s `kobo-cross-build` job do it in CI.
