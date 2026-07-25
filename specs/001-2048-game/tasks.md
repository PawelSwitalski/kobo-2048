# Tasks: 2048 Game

**Input**: Design documents from `/specs/001-2048-game/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: INCLUDED — Constitution v1.0.0 Principle III (non-negotiable) mandates host-run tests
for core logic (board slide/merge/spawn/game-over, swipe classification, and every persistence
round-trip + corruption case). UI/screen wiring tasks are validated via the simulator and
quickstart.md instead, matching this template family's existing convention (see sibling project
`kobo-sudoku`).

**Organization**: Tasks are grouped by user story, in the priority order from spec.md: US1 (P1) →
US2 (P2) → US3, US4, US5 (P3, spec order). This repo is a **template**, not an empty project — the
platform (`Renderer`/`TouchInput`), app shell, theme, widgets, and persistence primitives already
exist and work (proven by the placeholder tap-counter demo). Setup/Foundational are therefore
short: remove the placeholder, then let each story build only what it needs.

**Post-`/speckit-analyze` revision (2026-07-25)**: `BestScore` moved from US4 into US1 (in-memory
tracking from the start; only its JSON persistence stays in US4), so US2's "best score is
unchanged" acceptance scenario is actually observable when US2 is built. Tile-value legibility and
a Color-mode distinguishability pass were added explicitly. See the analysis findings F1/U1/E1 this
revision resolves.

**Post-implementation addendum (2026-07-25)**: User Story 6 (Exit the game, Priority P2) added
after a user-reported gap — no in-UI way to leave the app. Appended as Phase 9 (T040) rather than
renumbering the 39 already-completed tasks above; see plan.md's "Addendum: User Story 6" for why no
new architecture/tests are needed.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: US1 play & merge · US2 game over & restart · US3 win & keep playing · US4 resume ·
  US5 display settings · US6 exit the game

## Path Conventions

Single project at repository root per plan.md: `src/`, `tests/`, `docs/`.

---

## Phase 1: Setup

**Purpose**: Clear out the template's placeholder demo so the real feature has a clean slot

- [x] T001 Remove the placeholder tap-counter demo: delete `src/core/counter.h`,
      `src/core/counter.cpp`, `src/ui/screens/counter_screen.h`, `src/ui/screens/counter_screen.cpp`,
      `src/ui/screens/about_screen.h`, `src/ui/screens/about_screen.cpp`, `tests/test_counter.cpp`;
      remove their entries from `CMakeLists.txt`'s `kobo_2048_core`, `kobo_2048_ui`, and
      `kobo_2048_tests` source lists

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Shrink `ui::App` back to a minimal navigation-only surface so every story below adds
its own accessors without fighting the placeholder shape

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T002 In `src/ui/app.h`, remove the placeholder `counter()`/`autosave()` methods from the
      `App` interface, keeping `renderer()`, `theme()`, `push()`, `pop()`, `requestExit()`; in
      `src/main.cpp`'s `AppImpl`, remove the now-dangling `counter_` member, `counter()`/`autosave()`
      overrides, the `Counter::fromJson`/`toJson` load block, and the `push(CounterScreen)` call
      (each user story phase below adds back exactly the accessors and startup wiring it needs)

**Checkpoint**: Project builds (host tests target + simulator target) with an empty app shell —
user story implementation can now begin

---

## Phase 3: User Story 1 - Play and merge tiles (Priority: P1) 🎯 MVP

**Goal**: Swipe on the touchscreen to slide and merge tiles on a 4×4 board, in memory, with a
live current score and best score — the complete core loop, no persistence or notifications yet

**Independent Test**: In the simulator, swipe in each of the four directions from a new game;
confirm tiles slide, equal tiles merge exactly once per move, a new tile appears only when the
board changed, current score increases by each merge's value, and best score tracks the
highest current score reached so far in this run

### Tests for User Story 1 (Constitution III — write first, must fail, then implement)

- [x] T003 [P] [US1] Board tests in `tests/test_board.cpp`: `applyMove` slides+merges correctly in
      all 4 directions; a line of 3+ equal tiles merges exactly once, nearest the direction of
      movement (spec User Story 1 Scenario 2); a swipe with no legal move leaves the board
      byte-for-byte unchanged (Scenario 3); `spawnTile` only ever targets empty cells and produces
      value 2 in ~90% / value 4 in ~10% of trials over many seeded runs; `hasReached`/`isGameOver`
      correctness on hand-built fixtures (full-with-merge-available vs. truly stuck)
- [x] T004 [P] [US1] `classifySwipe` tests in `tests/test_gesture.cpp`: a `Tap` whose start/end
      delta is below the deadzone returns `nullopt`; each of the four directions is resolved
      correctly from `dx`/`dy`; the axis with the larger absolute delta wins on a diagonal gesture
- [x] T005 [P] [US1] `GameSession` tests in `tests/test_game_session.cpp`: `move()` applies the
      board change and adds the merged value to `score()` only when the board actually changed; a
      no-op move leaves `score()` unchanged; `justWon()` is true exactly once, on the move that
      first produces a 2048 tile, and `winShown()` stays true afterward for the rest of the session
- [x] T006 [P] [US1] `BestScore` tests in `tests/test_best_score.cpp`: `update(score)` only ever
      increases the stored value (a lower or equal incoming score leaves it unchanged); a
      freshly-constructed `BestScore` starts at 0

### Implementation for User Story 1

- [x] T007 [US1] Implement `BestScore` (`value`, `update(uint32_t)`) — value tracking only, no
      JSON yet — in `src/core/best_score.h`/`src/core/best_score.cpp` per contracts/core-model.md;
      register in `kobo_2048_core` and add `tests/test_best_score.cpp` to `kobo_2048_tests` in
      `CMakeLists.txt` (depends on: T006)
- [x] T008 [US1] Implement `Board` (16-cell row-major grid, `Direction`, `applyMove`, `spawnTile`,
      `hasReached`, `isGameOver`, `full`) in `src/core/board.h`/`src/core/board.cpp` per
      data-model.md and contracts/core-model.md; register both files in `kobo_2048_core` in
      `CMakeLists.txt` and add `tests/test_board.cpp` to `kobo_2048_tests` (depends on: T003)
- [x] T009 [US1] Implement `GameSession` (`newGame`, `move`, `score`, `winShown`, `justWon`) in
      `src/core/game_session.h`/`src/core/game_session.cpp` per contracts/core-model.md; register
      in `CMakeLists.txt` (`kobo_2048_core`, and `tests/test_game_session.cpp` in
      `kobo_2048_tests`) (depends on: T008, T005)
- [x] T010 [US1] Implement `classifySwipe(Tap, int minDistancePx) -> optional<Direction>` in
      `src/ui/gesture.h`/`src/ui/gesture.cpp` per contracts/platform-abstraction.md; register in
      `kobo_2048_ui` and add `tests/test_gesture.cpp` to `kobo_2048_tests` in `CMakeLists.txt`
      (depends on: T004)
- [x] T011 [US1] Extend `Tap` with `startX`/`startY` (down position) in `src/platform/input.h` per
      contracts/platform-abstraction.md; `startX == x`/`startY == y` for a stationary tap
- [x] T012 [P] [US1] `MouseTouch`: record the position on `SDL_MOUSEBUTTONDOWN`, report it as
      `startX`/`startY` alongside the existing release point on `SDL_MOUSEBUTTONUP` in
      `src/platform/sdl/mouse_touch.cpp` (depends on: T011)
- [x] T013 [P] [US1] `EvdevTouch`: record the first `ABS_MT_POSITION_X`/`Y` (or `ABS_X`/`Y`
      single-touch fallback) seen after touch-down as `startX`/`startY`, alongside the
      already-tracked release position, applying the same swap/mirror/scale mapping to both, in
      `src/platform/kobo/evdev_touch.h`/`src/platform/kobo/evdev_touch.cpp` (depends on: T011)
- [x] T014 [US1] Add in-memory `session()` and `bestScore()` accessors (no persistence yet) to
      `ui::App` in `src/ui/app.h`; in `src/main.cpp`'s `AppImpl`, add `GameSession session_`
      (constructed via `GameSession::newGame`) and `BestScore best_` members, call
      `best_.update(session_.score())` after every mutation, implement the accessors, and push a
      `GameScreen` instead of the removed `CounterScreen` (depends on: T002, T009, T007)
- [x] T015 [US1] Implement `GameScreen`: draw the 4×4 board (tile fills via existing `Gray` shades
      + printed value, satisfying FR-014's baseline before Color mode exists, with the font size
      scaled so values up to 5 digits — e.g. `65536` — stay fully visible within the tile rect, no
      clipping), draw current score and best score side-by-side, handle taps via `classifySwipe` →
      `session().move()` → `bestScore()`-aware redraw, partial-refresh the changed board region in
      `src/ui/screens/game_screen.h`/`src/ui/screens/game_screen.cpp`; register in `kobo_2048_ui`
      in `CMakeLists.txt` (depends on: T009, T010, T011, T012, T013, T014)

**Checkpoint**: Full core loop playable start-to-finish in the desktop simulator (and device),
current + best score both visible; game continues past game-over/2048 with no notification yet —
those are the next two stories

---

## Phase 4: User Story 2 - Detect game over and start a new game (Priority: P2)

**Goal**: A game-over notice when no moves remain, and a New Game control available at any time

**Independent Test**: Drive a board to a full grid with no equal adjacent pairs and confirm a
game-over notice appears and swipes stop doing anything; separately, trigger New Game at any
point and confirm the board/current score reset while best score (tracked in-memory since User
Story 1; persisted starting User Story 4) is unaffected

No new core logic — `Board::isGameOver()`, `GameSession::newGame()`, and `BestScore::update()`
already exist and are already tested (T003/T006/T008/T007). This story is UI wiring only.

### Implementation for User Story 2

- [x] T016 [US2] In `GameScreen`, after a move that doesn't change the board or leaves
      `session().board().isGameOver()` true, show a game-over dialog (reuse the existing `Dialog`
      widget) and stop routing swipes to `session().move()` until New Game in
      `src/ui/screens/game_screen.cpp` (depends on: T015)
- [x] T017 [US2] Add a New Game control (button, reachable during play and from the game-over
      dialog) that replaces `session_` with a fresh `GameSession::newGame` and redraws — `best_` is
      untouched by this reset — in `src/ui/screens/game_screen.h`/`src/ui/screens/game_screen.cpp`
      + `src/ui/app.h` (a `newGame()` method on `App`) (depends on: T015)

**Checkpoint**: A complete, replayable game session, with best score visibly unaffected by New
Game — matches spec User Story 2 acceptance scenarios

---

## Phase 5: User Story 3 - Win the game and keep playing (Priority: P3)

**Goal**: A one-time win notice at the first 2048 tile, then unrestricted continued play

**Independent Test**: Drive a board to produce a 2048 tile and confirm the win notice appears
exactly once, then confirm swiping continues to work normally with no upper tile-value limit

No new core logic — `GameSession::justWon()`/`winShown()` already exist and are already tested
(T005). This story is UI wiring only.

### Implementation for User Story 3

- [x] T018 [US3] In `GameScreen`, after a move where `session().justWon()` is true, show a win
      dialog (reuse `Dialog`) with a "Keep Playing" dismissal; `Board` already has no upper value
      cap, so play continues unrestricted afterward, in `src/ui/screens/game_screen.cpp` (depends
      on: T015)

**Checkpoint**: All three P1–P3 core-loop stories done — matches spec User Story 3

---

## Phase 6: User Story 4 - Resume progress after closing the app (Priority: P3)

**Goal**: The in-progress game and best score survive app close/reopen and interruption
(device sleep, power loss, forced close) with no data loss beyond the current in-flight move

**Independent Test**: Play a few moves, fully close the app, reopen it, and confirm the board,
current score, and best score are identical to what was left; separately, hand-corrupt `save.json`
and confirm a fresh game starts with no crash

### Tests for User Story 4 (Constitution III)

- [x] T019 [P] [US4] Extend `tests/test_game_session.cpp`: `GameSession::toJson()`/`fromJson()`
      round-trips `board`/`score`/`winShown` losslessly; malformed input (wrong-length board array,
      non-power-of-two cell value, negative/overflowing score, missing keys) throws
- [x] T020 [P] [US4] Extend `tests/test_best_score.cpp`: `BestScore::toJson()`/`fromJson()`
      round-trips `value` losslessly; malformed input throws (the "only increases" rule itself is
      already covered by T006 and unaffected by serialization)

### Implementation for User Story 4

- [x] T021 [US4] Implement `BestScore::toJson()`/`fromJson()` per contracts/save-format.md's
      `best.json` schema v1, in `src/core/best_score.cpp` (depends on: T020)
- [x] T022 [US4] Implement `GameSession::toJson()`/`fromJson()` exactly per
      contracts/save-format.md's `save.json` schema v1, in `src/core/game_session.cpp` (depends
      on: T019)
- [x] T023 [US4] Add `save`/`best` file paths (`dataDir/save.json`, `dataDir/best.json`) to
      `persist::Paths` in `src/persist/paths.h`/`src/persist/paths.cpp`
- [x] T024 [US4] Wire persistence into the app shell: at startup, load `save.json` into `session_`
      (missing/invalid ⇒ `GameSession::newGame`, FR-013) and `best.json` into `best_` (missing/
      invalid ⇒ zero); after every `GameScreen` move that changes the board, save `save.json`, and
      whenever `best_.update()` actually raises the value, save `best.json` too; keep the existing
      SIGTERM/exit path calling this same save routine, in `src/ui/app.h` (a `saveGame()` method) +
      `src/main.cpp` + `src/ui/screens/game_screen.cpp` (depends on: T021, T022, T023, T014, T015)

**Checkpoint**: quickstart.md scenarios 6 (close/reopen) and 7 (corrupt save) pass

---

## Phase 7: User Story 5 - Customize display settings (Priority: P3)

**Goal**: A Settings screen with a Color/Black & White theme (auto-detected, overridable with a
warning) covering the whole UI, and a full-refresh cadence control

**Independent Test**: Open Settings, switch color mode and confirm the whole UI (not just tiles)
updates; confirm first-launch color mode matches the device's detected panel capability; confirm a
warning appears choosing Color on a monochrome panel; change the full-refresh cadence and confirm
forced full refreshes occur at the new cadence

### Tests for User Story 5 (Constitution III)

- [x] T025 [P] [US5] `DisplaySettings` tests in `tests/test_display_settings.cpp`: round-trip is
      lossless; malformed input throws (defaults apply at the call site per FR-013's rule);
      `autoDetect(true)` yields `ColorMode::Color`, `autoDetect(false)` yields
      `ColorMode::BlackWhite`

### Implementation for User Story 5

- [x] T026 [US5] Implement `DisplaySettings` (`colorMode`, `fullRefreshEvery`, `toJson`,
      `fromJson`, `autoDetect`) in `src/core/display_settings.h`/`src/core/display_settings.cpp`
      per contracts/core-model.md; register in `kobo_2048_core` and add
      `tests/test_display_settings.cpp` to `kobo_2048_tests` in `CMakeLists.txt` (depends on: T025)
- [x] T027 [US5] Add a `settings` file path (`dataDir/settings.json`) to `persist::Paths` in
      `src/persist/paths.h`/`src/persist/paths.cpp` (depends on: T023)
- [x] T028 [US5] Extend `Renderer`'s `Color` enum with `ChromeBg`, `ChromePanel`, `ChromeButton`,
      `Tile1`..`Tile7`, `TileHigh` in `src/platform/renderer.h` per
      contracts/platform-abstraction.md
- [x] T029 [US5] Map each new `Color` value to an RGB constant in `CanvasRenderer::fillRect`
      (`src/platform/canvas_renderer.h`) and the matching text-color branch in
      `SoftCanvas::drawText` (`src/platform/softcanvas.cpp`) — the single shared location both
      backends draw through (depends on: T028)
- [x] T030 [US5] Implement a pure tile-value → color-tier mapping (caps at `TileHigh` for 256 and
      above, so arbitrarily high "keep going" tiles stay a bounded closed set) in
      `src/ui/tile_theme.h`/`src/ui/tile_theme.cpp`; register in `kobo_2048_ui` in
      `CMakeLists.txt` (depends on: T028)
- [x] T031 [US5] Extend `Theme`/theme selection to provide a Color-mode palette (background,
      score panel, buttons, tile fills) and keep the existing Black & White palette
      (`Gray` shades only, no new `Color` values — proves FR-014 by construction), selected by
      `DisplaySettings.colorMode`; apply it in `GameScreen::draw` in `src/ui/theme.h`/
      `src/ui/theme.cpp` + `src/ui/screens/game_screen.cpp` (depends on: T026, T029, T030)
- [x] T032 [US5] Implement `SettingsScreen`: color mode toggle (shows a warning dialog, reusing
      `Dialog`, when Color is chosen while `Renderer::info().color` is false, per FR-018) and a
      full-refresh cadence cycle button (`5 / 10 / 25 / Never`, calling
      `renderer.setGhostingInterval(n)`), Back button; add a way to reach it from `GameScreen` in
      `src/ui/screens/settings_screen.h`/`src/ui/screens/settings_screen.cpp` +
      `src/ui/screens/game_screen.cpp`; register in `kobo_2048_ui` in `CMakeLists.txt` (depends
      on: T026, T031)
- [x] T033 [US5] Wire settings persistence into the app shell: at first launch (no
      `settings.json`), set `colorMode` via `DisplaySettings::autoDetect(renderer.info().color)`
      (FR-017); load/save `settings.json` the same way as `save.json`/`best.json`; call
      `renderer.setGhostingInterval(settings.fullRefreshEvery)` once before the main loop starts,
      in `src/ui/app.h` + `src/main.cpp` (depends on: T026, T027, T032)

**Checkpoint**: quickstart.md scenarios 8, 9, 10, 12 pass — all five user stories functional

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Sync documentation, final Success Criteria verification, cleanup

- [x] T034 [P] Apply the `Tap`/`Color` delta from `specs/001-2048-game/contracts/platform-abstraction.md`
      back into the canonical `docs/contracts/platform-abstraction.md`
- [x] T035 [P] Rewrite `docs/settings.md`'s "no in-game settings screen" placeholder note to
      document the real Settings screen (color mode, full-refresh cadence) added in User Story 5
- [x] T036 [P] Update `README.md` and `SETUP.md`'s placeholder-demo description (tap-to-increment
      counter) to describe the 2048 game
- [x] T037 Grayscale/Black & White **and** Color-mode distinguishability audit (SC-006, SC-007):
      play a full game in Black & White mode confirming every value is identifiable by its printed
      number alone with no color-only signal anywhere in the UI; separately play a full game in
      Color mode confirming every tile-tier color remains visually distinct and numerals stay
      legible against every tile fill color
- [x] T038 Run all of quickstart.md's validation scenarios (1–14) end to end — host `ctest` plus
      simulator/device scenarios — and record results in quickstart.md
- [x] T039 Code cleanup: remove any leftover `counter.json`/placeholder references, confirm
      `CMakeLists.txt`'s `kobo_2048_tests` source list matches exactly the test files above, confirm
      no new third-party dependency was introduced (Constitution VI)

---

## Phase 9: User Story 6 - Exit the game (Priority: P2)

**Goal**: A one-tap way to leave the app from the game screen and return to the device's normal
home screen, instead of relying only on the idle-auto-exit timeout or an external signal

**Independent Test**: From the game screen, in any reachable state (fresh game, mid-game, win
dialog open, game-over dialog open), tap the Exit control in the top-right corner and confirm the
app closes, control returns to the device's normal home screen, and the board/score/best score are
unchanged the next time the app is opened

No new core logic — `ui::App::requestExit()` and the main loop's `exitRequested()` →
`saveOnExit()` → clean-return-to-Nickel handling already exist (used today by the idle-exit and
SIGTERM paths) and are unmodified by this story. This story is UI wiring only, so no test task,
consistent with the same no-new-core-logic pattern already used for User Stories 2 and 3.

### Implementation for User Story 6

- [x] T040 [US6] Add an Exit control to `GameScreen`'s top-right corner, with the "2048" title in
      the top-left (small `Button`, reusing the existing `Color::ChromeButton`/`Gray` theming
      already used by `newGameButton_`/`settingsButton_`; both share one touch-target-tall top row)
      in `src/ui/screens/game_screen.h`/`src/ui/screens/game_screen.cpp`: draw it in
      `draw()`; in `onTap()`, hit-test it **before** the modal-dialog branch so it stays tappable
      even while the win or game-over dialog is open (spec Edge Case); on hit, call
      `app_.requestExit()` directly with **no confirmation dialog** — matches the `Exit` pattern in
      the sibling project `kobo-sudoku`'s `src/ui/screens/menu_screen.cpp` (plain
      `app_.requestExit()` on tap, no discard-confirmation, since autosave already covers progress)
      (depends on: T015)

**Checkpoint**: quickstart.md's Exit scenario passes — matches spec User Story 6 acceptance
scenarios; all six user stories functional

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)** → **Foundational (Phase 2)** → all user stories
- **US1 (Phase 3)**: only needs Foundational; delivers the playable core loop plus in-memory best
  score (MVP)
- **US2 (Phase 4)**: extends US1's `GameScreen`; needs T015
- **US3 (Phase 5)**: extends US1's `GameScreen`; needs T015; independent of US2
- **US4 (Phase 6)**: serializes US1's `GameSession`/`BestScore`; needs T009/T007 (session/best
  score), T014/T015 (app wiring point); independent of US2/US3 logic but shares `GameScreen` edits
  with them (sequence, don't parallelize file edits across US2/US3/US4 on `game_screen.cpp`)
- **US5 (Phase 7)**: independent core (`DisplaySettings`) but its `persist::Paths` task (T027)
  follows US4's (T023) since they touch the same file; its theme work (T031) touches
  `game_screen.cpp` after US2/US3/US4's edits land
- **Polish (Phase 8)**: after all desired stories
- **US6 (Phase 9)**: added later; needs T015 (`GameScreen` exists); touches `game_screen.cpp` after
  every other story's edits to that file (sequence last, same file-conflict rule as US2–US5)

### Within Each User Story

- Test tasks first (write → watch fail → implement until green; Constitution III) where the story
  introduces new core logic
- Core (`src/core`) before platform/UI wiring; `GameScreen` wiring last in each story

### Parallel Opportunities

- Phase 3: T003, T004, T005, T006 in parallel (independent test files); T012, T013 in parallel
  once T011 lands (different backend files)
- Phase 6: T019, T020 in parallel
- Phase 7: T025 alone, then T027 depends on T023 (sequential, same file family); T028 can start any
  time; T029, T030 both depend only on T028
- Phase 8: T034, T035, T036 in parallel (independent doc files)
- `game_screen.cpp` is touched by US1, US2, US3, US4, US5, and US6 — treat edits to that one file
  as sequential across stories even where the story's other files could parallelize

## Parallel Example: User Story 1 kick-off

```bash
# Tests, all independent files:
Task: "Board tests in tests/test_board.cpp"
Task: "classifySwipe tests in tests/test_gesture.cpp"
Task: "GameSession tests in tests/test_game_session.cpp"
Task: "BestScore tests in tests/test_best_score.cpp"

# After T011 (Tap extended), backend updates in parallel:
Task: "MouseTouch down-tracking in src/platform/sdl/mouse_touch.cpp"
Task: "EvdevTouch down-tracking in src/platform/kobo/evdev_touch.cpp"
```

## Implementation Strategy

### MVP First

1. Phase 1 + Phase 2 (remove placeholder, minimal `App`)
2. Phase 3 (US1): full core loop playable in the simulator, current + best score both visible —
   **validate before anything else**
3. Stop and play a few games; this alone is a recognizable, if incomplete, "2048"

### Incremental Delivery

Each subsequent phase (US2 game-over/restart → US3 win → US4 resume → US5 display settings) is
independently testable per its Independent Test above and leaves the game shippable at every
checkpoint. Commit after each task or logical group.

---

## Notes

- Tests are mandatory only where Constitution III applies (new core logic); pure UI/screen wiring
  tasks are validated via the simulator and quickstart.md instead
- `[P]` = different files, no incomplete-task dependencies
- Keep `src/core` free of any OS/IO includes (Constitution I) — persistence goes through
  `src/persist`, rendering/input through `Renderer`/`TouchInput`
- This is a template repository: Setup/Foundational are deliberately thin (remove placeholder,
  trim `App`) rather than building the platform layer from scratch
- `BestScore` is split across two stories on purpose: US1 builds and tests its value-tracking rule
  in memory (so US2's "best score unaffected by New Game" is actually observable as soon as US2 is
  built); US4 adds only its JSON persistence
- US6 (T040) is a later addendum appended after the original six-story implementation shipped —
  task IDs are append-only (T040, not renumbered into priority order) to avoid disturbing the
  39 already-completed tasks above
