# Research: 2048 Game

**Date**: 2026-07-25 | **Feature**: [spec.md](./spec.md)

All NEEDS CLARIFICATION items from the Technical Context were resolved. Findings below. This
feature reuses the template's existing architecture (the same one already proven in the sibling
project `kobo-sudoku`), so most decisions are "keep what's already here"; only the items below
required new investigation.

## R1. Overall architecture: reuse the template as-is

**Decision**: No change to the template's layered architecture (`core` / `persist` / `platform` /
`ui`, FBInk+evdev device backend, SDL2 desktop simulator, koxtoolchain cross-build, KFMon+
NickelMenu launch, JSON persistence). This is the same stack already shipped in the sibling
project `kobo-sudoku`, which the user pointed to as a reference.

**Rationale**: The template was extracted from that exact working, on-device-proven stack
(constitution + `docs/contracts/platform-abstraction.md` + `docs/contracts/install-layout.md`
already encode it). Re-deriving it would violate Constitution VI (simplicity: no speculative
redesign of a working foundation).

**Alternatives considered**: None — re-litigating the platform stack is out of scope for a
feature plan; it is the template's job, already done.

## R2. Swipe gesture detection (FR-002/FR-003)

**Decision**: Extend the existing `Tap` struct (`platform/input.h`) with the touch **down**
position alongside the existing **up**/release position it already reports:

```cpp
struct Tap { int x = 0, y = 0; int startX = 0, startY = 0; };  // startX/Y == x/y for a plain tap
```

`MouseTouch` (SDL) starts tracking on `SDL_MOUSEBUTTONDOWN` and reports both points on
`SDL_MOUSEBUTTONUP`; `EvdevTouch` records the first `ABS_MT_POSITION_X/Y` (or `ABS_X/Y`) seen in a
down→up cycle as the start, alongside the already-tracked last position as the end. Both changes
are additive and backward-compatible: every screen that only reads `tap.x/tap.y` (button
hit-testing on Settings, About, etc.) is unaffected, since `startX/startY == x/y` for a stationary
tap.

A new **portable, host-testable** pure function in `ui` (not `platform`) classifies the gesture:

```cpp
enum class Direction { Up, Down, Left, Right };
std::optional<Direction> classifySwipe(Tap t, int minDistancePx);
```

It picks the axis with the larger absolute delta and returns its direction, or `nullopt` if
`max(|dx|,|dy|)` is below `minDistancePx` (a DPI-scaled minimum swipe distance, e.g. `theme.mm(8.0)`,
matching this codebase's existing DPI-relative-everything convention). `GameScreen::onTap` calls
this instead of doing hit-testing.

**Rationale**: The current `TouchInput` contract (used unchanged by every other screen in this
template family) only ever reports the *release* point — both backends already collapse a
down/up cycle into one `Tap` before the UI ever sees it, discarding the start position entirely.
That is sufatisfies plain taps but cannot express a direction. The smallest change that adds
direction is to stop discarding the start point; classifying it into a `Direction` is then pure
arithmetic with no platform dependency, so it belongs in `ui` where it can be unit-tested on the
host (Constitution I/III) instead of being buried in `EvdevTouch`/`MouseTouch`.

**Alternatives considered**:
- *New parallel `waitForSwipe()` method on `TouchInput`*: rejected — duplicates the whole
  down/up state machine already implemented once in each backend, doubling the code that must be
  correct in `EvdevTouch` (the fiddlier of the two, raw evdev parsing) for no behavioral gain over
  augmenting the existing `Tap`.
- *Velocity/fling-based gesture recognition*: rejected as over-engineering — 2048 only needs a
  discrete direction, not swipe speed; a distance threshold is simpler (Constitution VI) and
  matches how the reference game treats any swipe past a small deadzone as a full move.
- *Reading raw evdev events directly in `ui`*: rejected — breaks Constitution I (platform code
  must stay behind the two interfaces).

## R3. Color-mode tile/theme rendering (FR-016, FR-018, User Story 5)

**Decision**: Extend the `Renderer` contract's `Color` enum (`platform/renderer.h`, currently just
`{None, Red}`) with the small, closed set of named accents this feature needs — one per tile
"tier" plus the Color-mode chrome accents (background/panel/button), e.g.:

```cpp
enum class Color : uint8_t {
    None, Red,                                   // existing
    ChromeBg, ChromePanel, ChromeButton,          // Color-mode background/score-panel/buttons
    Tile1, Tile2, Tile3, Tile4, Tile5, Tile6, Tile7, TileHigh,  // tile value tiers
};
```

The RGB mapping for each new value is added in exactly **one** place —
`CanvasRenderer::fillRect` (`platform/canvas_renderer.h`) and the matching branch in
`SoftCanvas::drawText` (`platform/softcanvas.cpp`) — because both backends already share this one
software rasterizer and only differ in how they flush it to screen/framebuffer. Neither
`FbinkRenderer` nor `SdlRenderer` nor `EvdevTouch`/`MouseTouch` needs to change.

A small portable mapping in `core` (or `ui`) reduces an arbitrary tile value to one of a fixed
number of tiers (e.g. tiers for 2, 4, 8, 16, 32, 64, 128, and "256 and above"), so tile colors
stay a bounded, closed set no matter how high a tile's value climbs after "keep going" — mirroring
how the reference game's own stylesheet caps out with a "super" tile class past 2048.

Black & White mode does not use any new `Color` value: it continues to rely purely on `Gray`
shades (already present) plus the printed number, per the existing FR-014.

**Rationale**: `Color::Red` already proves this exact pattern works end-to-end (one enum value,
one RGB constant, shared by both backends) for the one accent this template currently has; adding
more named values is the same mechanism, not a new one. Because the mapping lives in the shared
`CanvasRenderer`/`SoftCanvas`, this stays a single, host-buildable, host-testable-at-the-UI-layer
change — no e-ink or SDL specifics involved.

**Alternatives considered**:
- *Generic truecolor (`struct Rgb{uint8_t r,g,b;}`) instead of an enum*: rejected — the palette is
  a small fixed set known at design time (tile tiers + three chrome accents); a closed enum keeps
  Color-mode assets reviewable in one place and keeps Black & White mode's guarantee (FR-014, no
  meaning encoded in color alone) easy to audit, per Constitution VI (no speculative generality).
- *Per-backend color tables (FBInk vs. SDL)*: rejected — would duplicate the mapping and risk the
  two backends drifting visually, defeating the "pixel-identical rendering between simulator and
  device" property `SoftCanvas` exists to guarantee.

## R4. Full-refresh cadence setting (FR-019, FR-020)

**Decision**: Reuse the ghosting-interval mechanism that already exists in this template's
`Renderer` contract — `virtual void setGhostingInterval(int n)`, already implemented in
`FbinkRenderer` (`ghostingPartials_`, default 12) and a no-op default on backends without a
ghosting concept (the SDL simulator). Add a Settings screen that cycles the same option scale
already proven in the sibling project (`5 / 10 / 25 / Never`, where `Never` maps to `n <= 0`),
calling `renderer.setGhostingInterval(n)` on change and applying the persisted value again at
startup.

**Rationale**: This is exactly the `kobo-sudoku` "Screen refresh every" setting the user pointed
to as a reference, and the platform hook for it is already scaffolded in this template
(`docs/contracts/platform-abstraction.md`'s "User-tunable ghosting policy" note) — no new
platform-layer work, just the Settings UI and persistence wiring `kobo-sudoku`'s
`SettingsScreen`/`settings.json` already demonstrate.

**Alternatives considered**: A different option scale — rejected; reusing the sibling project's
proven scale avoids re-litigating a UX question the user has already answered by pointing at it.

## R5. Color-mode default and manual override (from `/speckit-clarify`)

**Decision**: At first launch (no `settings.json` yet), read `Renderer::info().color`
(`DisplayInfo.color`, already populated by every backend) and set the persisted color mode to
`Color` if true, `BlackWhite` if false. The Settings screen always offers both options regardless
of `DisplayInfo.color`; selecting `Color` while `DisplayInfo.color` is false shows a one-line
warning ("Colors may not display correctly on this screen") without blocking the choice.

**Rationale**: `DisplayInfo.color` already exists precisely for this kind of decision (theme.h's
own `bool color` mirrors it) — no new platform capability needed. This directly implements the
clarified answer (auto-detect + preselect + manual override + warning) with existing primitives.

**Alternatives considered**: Restricting the Settings option outright on non-color panels —
explicitly rejected in `/speckit-clarify` in favor of the warn-but-allow approach.

## R6. Persistence layout

**Decision**: Three JSON files under the app's data directory (mirrors `kobo-sudoku`'s
`save.json` / `stats.json` / `settings.json` three-way split, and this template's existing
`persist::Paths` pattern):

- `save.json` — the single in-progress `GameSession` (board, current score, win-shown flag).
  Overwritten on every completed move; replaced (not merged) by "New Game".
- `best.json` — the single persistent `bestScore` integer. Survives "New Game"; only ever
  increases.
- `settings.json` — `DisplaySettings` (color mode, full-refresh cadence). Independent of any
  game.

Each uses the existing `persist::saveFileAtomic`/`loadFile` (write-temp-then-rename) and the same
"unreadable/invalid ⇒ treated as absent, defaults apply, never crash" rule already implemented for
`counter.json` and proven in `kobo-sudoku`.

**Rationale**: Matches FR-008 ("best score... persisted independently of any single game
session"), FR-012/FR-013, and FR-020, using infrastructure this template already has
(`persist/store.h`, `persist/paths.h`) unmodified. Splitting into three files (vs. one combined
file) keeps "New Game" a clean, independent reset of exactly `save.json` without touching best
score or settings, and keeps each file's schema small and independently versionable — the same
tradeoff `kobo-sudoku` already made for the identical shape of problem (session vs. cross-session
stats vs. settings).

**Alternatives considered**: One combined `save.json` holding session+best+settings — rejected:
"New Game" would need to carefully preserve two of three top-level keys while resetting the third,
more error-prone than three independently-atomic files for no size benefit at this scale.

## R7. Core game logic

**Decision**: A portable `core::Board` (16-cell array of `uint32_t` values, 0 = empty) with pure
functions/methods: `applyMove(Direction) -> MoveResult{changed, scoreGained}` (slide + merge, one
merge per tile per move, per FR-003/FR-004), `spawnTile(rng)` (2 @ 90% / 4 @ 10% into a random
empty cell, FR-005), `hasReached(2048)` (FR-009), `isGameOver()` (full grid, no equal orthogonal
neighbors, FR-010). A `core::GameSession` wraps `Board` + running score + win-shown flag and
exposes JSON `toJson`/`fromJson` (throws on invalid input, same contract as `Counter`).

**Rationale**: Directly mirrors this template's existing `core::Counter` shape (the shape
`ui/app.h` explicitly documents as the pattern to follow) and `kobo-sudoku`'s `core::Session` —
pure logic, no OS calls, unit-testable on host per Constitution III, with the RNG injected
(seedable) so merge/spawn behavior is deterministically testable.

**Alternatives considered**: None of note — this is the standard, well-understood 2048 algorithm;
the only real design choice (gesture input) is covered in R2.

## Sources

- This repository's own `docs/contracts/platform-abstraction.md`, `docs/contracts/install-layout.md`,
  and `.specify/memory/constitution.md` — authoritative for the architecture reused here.
- Sibling project `kobo-sudoku` (a separate local repository), named by the user as a
  reference: `src/ui/screens/settings_screen.{h,cpp}`, `src/ui/theme.h`,
  `specs/001-kobo-sudoku/{plan,research,data-model}.md` and its `contracts/` — confirms the
  settings-screen pattern, the three-file persistence split, and the portable-core/thin-platform
  split all reused here.
- [gabrielecirulli/2048](https://github.com/gabrielecirulli/2048) — reference game rules (slide,
  merge-once-per-move, 90/10 spawn odds, win-at-2048-then-continue), already encoded in `spec.md`.
