# Implementation Plan: 2048 Game

**Branch**: `001-2048-game` | **Date**: 2026-07-25 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/001-2048-game/spec.md`

## Summary

A native, fully offline recreation of [gabrielecirulli/2048](https://github.com/gabrielecirulli/2048)
for Kobo e-readers: swipe to slide and merge tiles on a 4×4 grid, score tracking with a persisted
best score, win notification at 2048 with "keep going," game-over detection, new-game restart, and
full resume of an in-progress game across app close/reopen or interruption. Adds two settings not
in the base reference game: a Color/Black & White display theme (auto-detected from the panel,
manually overridable) covering the entire UI, and a full-refresh cadence control.

Technical approach (from [research.md](./research.md)): reuse this template's existing
architecture unchanged — a portable C++17 core behind the `Renderer`/`TouchInput` interfaces, FBInk
+evdev device backend, SDL2 desktop simulator, koxtoolchain cross-build, KFMon+NickelMenu launch,
atomic-write JSON persistence — the same stack already proven in the sibling project
`kobo-sudoku`. Two small, additive extensions to the platform contract are required: `Tap` gains
the touch-down position (enables swipe-direction classification, done in portable `ui` code, not
in the platform backends) and `Renderer::Color` gains a small closed set of named accents for
Color-mode tile/chrome rendering (mapped once in the shared `CanvasRenderer`/`SoftCanvas`, so
neither backend needs its own color table).

## Technical Context

**Language/Version**: C++17

**Primary Dependencies**: FBInk (e-ink framebuffer rendering, vendored/static), nlohmann/json
(header-only, persistence), doctest (header-only, tests), SDL2 (host-only, desktop simulator).
Device input via raw evdev — no library. (Identical to this template's existing approved
dependency set; no new dependency introduced.)

**Storage**: JSON files (`save.json`, `best.json`, `settings.json`) in `.adds/kobo_2048/` on
device internal storage; atomic write-then-rename (existing `persist/store.h`, unchanged)

**Testing**: doctest unit tests for the core, built/run on the host (Windows native or WSL2);
manual on-device/simulator validation scenarios in [quickstart.md](./quickstart.md)

**Target Platform**: Kobo e-readers running stock firmware (Linux/armhf, e-ink framebuffer),
firmware 4.x and 5.x, monochrome and color (Kaleido) panels. Dev host: Windows 11 + WSL2/Docker
for cross-compilation

**Project Type**: Embedded/desktop-style native application (single binary, full-screen, touch)

**Performance Goals**: Swipe-to-visible-board-update < 1 s (target < 250 ms via partial e-ink
refresh, SC-002); save-after-move imperceptible (< 50 ms)

**Constraints**: Fully offline (FR-015); grayscale-first rendering with color strictly
accent/theme-only, never the sole carrier of meaning even in Color mode (FR-014, Constitution II);
e-ink refresh discipline (partial refreshes + full refresh on transitions + user-tunable forced
full-refresh cadence, FR-019); modest CPU/RAM; installable by plain USB file copy; must exit
cleanly back to Nickel

**Scale/Scope**: Single user, one in-progress game at a time; 4×4 board only; screens: game,
settings (+ win/game-over notifications as modal dialogs, reusing the existing `Dialog` widget);
~3–5 KLOC estimated (smaller than `kobo-sudoku` — no puzzle generator/solver)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Evaluated against Constitution v1.0.0 (ratified 2026-07-25):

| Principle | Status | Evidence in this plan |
|-----------|--------|------------------------|
| I. Portable core, thin platform layer | ✅ Pass | `core::Board`/`GameSession`/`DisplaySettings`/`BestScore` are OS-free ([contracts/core-model.md](./contracts/core-model.md)); the only platform-layer changes are additive fields/enum values behind the existing `Renderer`/`TouchInput` interfaces ([contracts/platform-abstraction.md](./contracts/platform-abstraction.md)); swipe classification is a pure `ui` function, not platform code (research R2) |
| II. E-ink-first, grayscale-first UX | ✅ Pass | No animation (spec Assumptions); Black & White mode uses only `Gray` + numerals, proving FR-014 by construction (research R3); Color mode is theme/accent, never the sole distinguisher; DPI-relative swipe deadzone, no hardcoded pixels |
| III. Host-testable correctness (NON-NEGOTIABLE) | ✅ Pass | `core::Board` slide/merge/spawn/game-over logic, `classifySwipe`, and all three persistence round-trips + corruption recovery are host-unit-tested (`tests/`, quickstart scenario 1) |
| IV. Firmware-agnostic device integration | ✅ Pass | No libnickel; FBInk + evdev + KFMon/NickelMenu only, unchanged from the template; one armhf binary; USB file-copy install ([docs/contracts/install-layout.md](../../docs/contracts/install-layout.md), unmodified by this feature) |
| V. Never lose the user's progress | ✅ Pass | `save.json` written after every board-changing move (atomic temp+rename); corrupt/missing file ⇒ fresh game, never a crash (FR-013, data-model.md GameSession state transitions) |
| VI. Simplicity and minimal dependencies | ✅ Pass | Zero new third-party dependencies; the two platform-contract extensions are minimal, additive, and justified against the alternatives rejected in research.md (R2, R3) |

**Post-Phase-1 re-check**: Design introduces no multi-project sprawl and no speculative
abstractions beyond the two platform interfaces Principle I already allows. The `Color` enum
extension and `Tap` start-point addition are the only contract changes, both scoped tightly to
this feature's needs (research.md R2/R3) and reviewed against simpler alternatives. Pass.

## Project Structure

### Documentation (this feature)

```text
specs/001-2048-game/
├── plan.md              # This file (/speckit-plan command output)
├── research.md          # Phase 0 output
├── data-model.md         # Phase 1 output
├── quickstart.md         # Phase 1 output
├── contracts/            # Phase 1 output
│   ├── core-model.md            # Board/GameSession/BestScore/DisplaySettings API
│   ├── platform-abstraction.md  # Delta: Tap start-point, Color enum extension
│   └── save-format.md           # JSON schemas for save.json/best.json/settings.json
└── tasks.md              # Phase 2 output (/speckit-tasks — NOT created by /speckit-plan)
```

### Source Code (repository root)

```text
src/
├── core/                     # Portable, no OS calls, fully unit-tested
│   ├── board.{h,cpp}         # 4x4 grid, slide+merge, spawn, game-over/win checks
│   ├── game_session.{h,cpp}  # Score, winShown, JSON round-trip (replaces core/counter.{h,cpp})
│   ├── best_score.{h,cpp}    # Persistent best score
│   └── display_settings.{h,cpp}  # colorMode + fullRefreshEvery, JSON round-trip, autoDetect()
├── persist/
│   ├── store.{h,cpp}         # Unchanged: atomic JSON load/save
│   └── paths.{h,cpp}         # Extended: save.json / best.json / settings.json paths
├── ui/                       # Renderer-agnostic screens & widgets
│   ├── theme.{h,cpp}         # Extended: Color-mode vs. Black & White palette selection
│   ├── widgets.{h,cpp}       # Unchanged (Button/Label/Dialog reused for win/game-over/warning)
│   ├── gesture.{h,cpp}       # New: classifySwipe(Tap, minDistancePx) -> optional<Direction>
│   └── screens/
│       ├── game_screen.{h,cpp}      # Replaces counter_screen.{h,cpp}; board rendering, swipe handling, win/game-over dialogs
│       └── settings_screen.{h,cpp}  # Replaces about_screen.{h,cpp}; color mode + refresh cadence
├── platform/
│   ├── renderer.h             # Modified: Color enum extended (research R3)
│   ├── input.h                 # Modified: Tap gains startX/startY (research R2)
│   ├── canvas_renderer.h       # Modified: new Color→RGB mappings (single shared location)
│   ├── softcanvas.{h,cpp}      # Modified: text-color branch for new Color values
│   ├── kobo/                   # Modified: EvdevTouch tracks touch-down position
│   └── sdl/                    # Modified: MouseTouch tracks SDL_MOUSEBUTTONDOWN position
└── main.cpp                    # Modified: wires GameSession/BestScore/DisplaySettings, calls
                                 # setGhostingInterval(settings.fullRefreshEvery) at startup

tests/                          # doctest, host-built
├── test_board.cpp              # slide/merge/spawn property tests (many seeds), game-over/win
├── test_gesture.cpp            # classifySwipe: deadzone, all four directions, diagonals
├── test_game_session.cpp       # score/winShown transitions, JSON round-trip, corruption
├── test_best_score.cpp         # round-trip, corruption, monotonic-increase rule
└── test_display_settings.cpp   # round-trip, corruption, autoDetect()

docs/
├── contracts/platform-abstraction.md   # Updated to match specs/001-2048-game/contracts/ delta
└── settings.md                          # Updated: document the new in-game Settings screen

third_party/, dist/, tools/, cmake/     # Unchanged by this feature
```

**Structure Decision**: Single project, single binary — unchanged from the template. This feature
removes the placeholder `core::Counter`/`CounterScreen`/`AboutScreen` (per `SETUP.md`'s own
guidance to delete them once real state exists) and replaces them with the entities and screens
above. The only architectural seam remains `platform/` (`Renderer`/`TouchInput`); this feature
extends both of its data shapes (delta above) but adds no new seam.

## Complexity Tracking

*No entries — Constitution Check passed with no violations requiring justification.*

## Addendum: User Story 6 - Exit the game (2026-07-25)

Post-implementation gap found by the user: the game screen has no UI-triggered way to leave the
app (only the existing idle-auto-exit timeout or SIGTERM). Added to spec.md as User Story 6
(Priority P2), FR-021, SC-009, and a new Edge Case (Exit while a win/game-over dialog is open).

**Technical approach**: No new architecture, no new files, no contract changes. `ui::App` already
declares `requestExit()` (used today only by `main.cpp`'s idle-exit/SIGTERM paths); this addendum
is exactly one new tappable control in the already-existing `GameScreen` that calls the
already-existing `app_.requestExit()`. (Placement corrected 2026-07-25: top-right corner, with the
"2048" title in the top-left — see the follow-up note at the end of this addendum.)
The main loop's existing `app.exitRequested()` check and `saveOnExit()` call already handle the
rest (Constitution V — no new persistence logic needed, FR-021's "no additional data loss" is
satisfied by code that already exists).

**Constitution re-check**: No principle affected beyond what's already evidenced in the table
above. Principle IV ("app MUST exit cleanly... return control to Nickel") is *better* satisfied
than before — the app previously depended entirely on an idle timeout or an external signal for
a clean exit; a direct in-UI affordance is a strict improvement, not a new risk. No Complexity
Tracking entry needed.

**Layout note**: `GameScreen`'s current layout has the title text ("2048") occupying the top-left;
the Exit control needs to coexist with it. Options: (a) a small icon/text button to the immediate
left or right of the title on the same row, (b) shrink the title and place Exit as a distinct
top-left button with the title shifted right. Left to `/speckit-tasks`' implementation task to
decide against the live `Theme` metrics (DPI-relative sizing, Constitution II) — not a design
decision requiring research.md, since it reuses widgets (`Button`) and mechanisms
(`requestExit()`) that already exist and are already proven.

**Task impact**: One new task in `tasks.md` (User Story 6, after the existing five), touching only
`src/ui/screens/game_screen.{h,cpp}` — no test task, since `requestExit()`/the main loop's exit
handling are not new code and only UI wiring is added, consistent with the same
no-new-core-logic-no-test-task pattern already used for User Stories 2 and 3.

**Follow-up correction (2026-07-25)**: after implementation, the user asked for the placement to
be swapped — Exit in the top-right corner, "2048" title in the top-left (the reverse of the
initial placement). Pure layout change in `GameScreen::layout()`: the title and Exit button trade
horizontal position within the same shared top row; no other behavior, file, or interface changed.
`spec.md` (User Story 6, FR-021), `tasks.md` (T040), and `quickstart.md` (scenario 15) updated to
match.
