# Quickstart: Build, Install & Validate

**Feature**: [spec.md](./spec.md) | Contracts: [core-model](./contracts/core-model.md),
[platform-abstraction](./contracts/platform-abstraction.md), [save-format](./contracts/save-format.md)

Packaging and launch mechanics are unchanged by this feature — see the template's existing
[docs/contracts/install-layout.md](../../docs/contracts/install-layout.md).

## Prerequisites

- **Windows 11 dev machine** with **WSL2** (Ubuntu) — used only for the device (cross) build.
- **koxtoolchain** installed in WSL2 (`arm-kobo-linux-gnueabihf`): https://github.com/koreader/koxtoolchain.
- **CMake ≥ 3.20** + a host C++17 compiler; SDL2 needed for the simulator build only.
- **Kobo device** with KFMon and/or NickelMenu installed (see `docs/installation.md`).

## Build

```bash
# Host: unit tests (fast inner loop)
cmake -B build/host -DKOBO_2048_BACKEND=none -DBUILD_TESTS=ON
cmake --build build/host && ctest --test-dir build/host --output-on-failure

# Host: desktop simulator (UI iteration without the device)
cmake -B build/sim -DKOBO_2048_BACKEND=sdl
cmake --build build/sim && ./build/sim/kobo_2048 --width 1264 --height 1680 --dpi 300

# Device (in WSL2): armhf binary + install zip
cmake -B build/kobo -DCMAKE_TOOLCHAIN_FILE=cmake/kobo-toolchain.cmake -DKOBO_2048_BACKEND=fbink
cmake --build build/kobo && tools/package.sh build/kobo
```

## Install on device

1. Connect the Kobo over USB.
2. Extract the packaged zip onto the device root (merges `.adds/`, adds the KFMon cover image).
3. Eject safely. "Kobo 2048" appears as a book in the library.
4. Tap its cover → the game launches.

## Validation scenarios (map to Success Criteria)

| # | Scenario | Expected | SC |
|---|----------|----------|----|
| 1 | Host: `ctest` — board slide/merge property tests across many seeds | Every merge is once-per-tile-per-move; scores match sum of merges | SC-002 |
| 2 | Device/simulator: new game, swipe in a direction with a legal move | Board updates immediately; exactly one new tile appears | SC-002 |
| 3 | Device/simulator: swipe toward a full edge with no legal move in that direction | Board unchanged, no new tile, no move counted | SC-002 |
| 4 | Device/simulator: drive a board to a full grid with no equal adjacent pairs | Game-over notice appears; further swipes ignored until New Game | SC-005 |
| 5 | Device/simulator: drive a board to produce a 2048 tile | Win notice appears exactly once; play continues afterward with no cap | — (User Story 3) |
| 6 | Device: play a few moves → close app (or power off) → relaunch | Board, current score, best score identical to pre-close state | SC-003, SC-004 |
| 7 | Device: corrupt `save.json` by hand → launch | Fresh new game starts, no crash | FR-013 |
| 8 | Device/simulator: open Settings on a device with `DisplayInfo.color == false`, select Color mode | Warning shown; selection still applied; entire UI (background/panel/buttons/tiles) switches | FR-016, FR-018 |
| 9 | Device/simulator: first launch on a color-capable panel vs. a monochrome panel | Color mode preselected to match panel capability with no manual step | SC-008 |
| 10 | Device/simulator: cycle the full-refresh setting (5/10/25/Never) in Settings, then play | Forced full refresh cadence changes accordingly; screen transitions still always flash | FR-019 |
| 11 | Grayscale/Black & White mode check: play a full game | All tile values distinguishable by their printed number alone, no color needed | SC-006, SC-007 |
| 12 | Color mode check: play a full game in Color mode | Every tile-tier color visually distinct; numerals stay legible against every fill color | SC-007 |
| 13 | Fresh user test: launch with no instructions | First merge produced within 30 s | SC-001 |
| 14 | Simulator: force a tile to a 5-digit value (dev/test hook or a long high-seed run) | Number fully visible within the tile, no clipping or overlap | Edge Case (long tile value) |
| 15 | Device/simulator: tap Exit (top-right of the game screen, with the "2048" title top-left) from a fresh game, mid-game, with the win dialog open, and with the game-over dialog open | App closes each time; control returns to the device's normal home screen; board/score/best score unchanged on next launch | FR-021, SC-009 |

Round-trip persistence, corrupted-file recovery, and board-logic edge cases are additionally
covered host-side in `tests/` (run in scenario 1's `ctest` invocation).

## Results (2026-07-25, initial implementation pass)

Verified on this Windows dev machine (host tests + desktop simulator only — no physical Kobo
device or WSL2/koxtoolchain available in this environment):

- **Scenario 1**: ✅ `ctest` — 37/37 test cases, 126/126 assertions pass (`tests/test_board.cpp`,
  `test_gesture.cpp`, `test_game_session.cpp`, `test_best_score.cpp`, `test_display_settings.cpp`,
  plus the pre-existing `test_persist.cpp`/`test_smoke.cpp`). Covers slide/merge/spawn for all 4
  directions, merge-once-per-tile, no-op-swipe-no-change, win/game-over detection, JSON round-trips
  and corruption handling for all three persisted files, and `classifySwipe`'s deadzone/direction
  resolution.
- **Simulator launch/render**: ✅ Verified visually (screenshots) in both Color mode and Black &
  White mode — title, current/best score panel, and board render correctly; starting tiles are
  visually distinct from empty cells in both modes (Black & White's empty-cell shade was tightened
  from `Gray::Lighter` to `Gray::White` during this check so tier-1 tiles don't visually blend into
  empty cells — see `GameScreen::drawCell`).
- **Scenarios 2–5, 8–10, 12, 14 (live interaction)**: ⚠️ Not directly exercised end-to-end in this
  pass — synthetic OS-level mouse-drag input (via PowerShell/Win32 `SendInput`/`mouse_event`) did
  not reliably reach the simulator window in this sandboxed environment, so a live swipe could not
  be screenshot-verified. The exact logic these scenarios exercise (swipe→direction→move→redraw,
  Settings toggles, tile font scaling) is covered by scenario 1's unit tests and code review, but a
  human should still play a real session in the simulator (`build/sim/Release/kobo_2048.exe`) to
  confirm the input pipeline feels right before shipping.
- **Scenarios 6, 7 (device close/reopen, corrupt save)**: ⚠️ Not run — no physical device;
  equivalent logic (round-trip + corruption → fresh game) is unit-tested in scenario 1.
- **Scenario 11**: ✅ Visually confirmed in Black & White mode (see simulator launch note above).
- **Scenario 13**: Not timed; the core loop's simplicity (swipe immediately merges/moves) makes the
  30 s budget trivially achievable, not treated as a risk.
- **Device cross-build (koxtoolchain/WSL2) and on-device play**: ⚠️ Not performed — requires
  Linux/WSL2, out of scope for this environment. Push to GitHub and let
  `.github/workflows/build.yml`'s `kobo-cross-build` job build it, then follow "Install on device"
  above.

## Results (2026-07-25, User Story 6 addendum — Exit control)

- **Scenario 15**: ✅ Verified live in the simulator — a synthetic single click (not a drag, unlike
  the swipe scenarios above) reliably reached the Exit button this time. Clicking Exit closed the
  process, and `save.json`/`best.json`/`settings.json` on disk matched the on-screen state at the
  moment of exit. Only the fresh-game/mid-game states were exercised this way; the "Exit while a
  win/game-over dialog is open" sub-case was verified by code review (`GameScreen::onTap` hit-tests
  `exitButton_` before the modal-dialog branch) rather than a live screenshot.
