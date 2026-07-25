# Data Model: 2048 Game

**Feature**: [spec.md](./spec.md) | **Plan**: [plan.md](./plan.md)

All entities live in the portable core (`src/core`, `src/persist`). Persistent entities serialize
to JSON per [contracts/save-format.md](./contracts/save-format.md).

## Tile (runtime value, part of Board)

A cell's value: `0` (empty) or a power of 2 starting at `2`. Not a separate object — `Board` is
simply 16 such values; a tile has no identity beyond "the value at this cell right now" (FR-001).

## Board (runtime, part of GameSession)

| Field | Type | Notes |
|-------|------|-------|
| `cells` | 16 × uint32 (4×4, row-major) | `0` = empty; otherwise a power of 2 |

**Rules**:
- `applyMove(Direction d)`: every non-empty cell slides as far as possible toward edge `d`;
  equal-value cells that collide merge into one cell of double value, processed in slide order so
  a tile born of a merge cannot merge again in the same call (FR-003/FR-004). Returns whether the
  board changed and the total value gained from merges this move (FR-006/FR-007).
- `spawnTile(rng)`: sets one randomly chosen empty cell to `2` (90%) or `4` (10%) (FR-005). No-op
  (should not be called) if no empty cell exists.
- `hasReached(value)`: `∃` cell `== value` — used with `2048` for win detection (FR-009).
- `isGameOver()`: **derived, not stored** — true iff every cell is non-empty and no two
  horizontally/vertically adjacent cells share a value (FR-010).

## GameSession (persistent: `save.json`)

The single in-progress game (one at a time, matching the counter/session-per-app-instance shape
this template already uses).

| Field | Type | Notes |
|-------|------|-------|
| `board` | Board | Embedded (16 cells) |
| `score` | uint32 | Current score; increases by the value of each merge (FR-007) |
| `winShown` | bool | Set the first time a cell reaches 2048 in this game; prevents re-showing the win notification on resume (FR-009, User Story 3 Scenario 2) |
| `schemaVersion` | uint | `1` |

**State transitions**:

```
(no save) ──New Game──▶ InProgress (two starting tiles, score 0, winShown false)
InProgress ──valid swipe (board changes)──▶ InProgress; persisted (FR-012)
InProgress ──swipe with no legal move──▶ InProgress, unchanged; NOT persisted (FR-006)
InProgress ──cell reaches 2048, winShown was false──▶ InProgress, winShown ⇒ true; win notice shown once (FR-009)
InProgress ──board full, no adjacent equal pair (isGameOver() true)──▶ game-over notice shown; further swipes ignored until New Game (FR-010)
InProgress ──New Game──▶ old save discarded, fresh InProgress (FR-011)
Corrupt/missing save.json ──▶ treated as (no save): a fresh game starts (FR-013)
```

`isGameOver` and the win check are both re-derived from `board`/`winShown` on every load and after
every move — nothing about "is this game over" is separately persisted, avoiding a second source
of truth that could desync from the board.

**Save policy**: written (atomic temp+rename) after every move that changes the board — bounds a
power-loss/interruption data loss to at most the current in-flight move (FR-012, SC-003/SC-004,
Edge Case: interrupted mid-move).

## BestScore (persistent: `best.json`)

| Field | Type | Notes |
|-------|------|-------|
| `value` | uint32 | Highest `GameSession.score` ever reached (FR-008) |
| `schemaVersion` | uint | `1` |

**Update rule**: whenever `GameSession.score` is persisted, if it exceeds the current `best.json`
value, `best.json` is updated too (same atomic-write policy). Never decreases; unaffected by "New
Game" (FR-011).

## DisplaySettings (persistent: `settings.json`)

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `colorMode` | enum `Color \| BlackWhite` | auto-detected from `DisplayInfo.color` at first launch (FR-017) | Applies to the whole UI theme — background, score panel, buttons, and tile fills (FR-016) |
| `fullRefreshEvery` | int | `12` (this template's existing `FbinkRenderer` default) | `0` = "Never"; otherwise the partial-refresh count before a forced full refresh (FR-019); matches the `5 / 10 / 25 / Never` option scale from the sibling project |
| `schemaVersion` | uint | `1` | |

Unknown fields are preserved on rewrite where practical; an unreadable/invalid file is treated as
absent — defaults (including the auto-detect step) apply as if this were the first launch
(consistent with FR-013's rule for all persistent files).

## Derived/transient (never persisted)

- **Direction**: `Up | Down | Left | Right` — the outcome of classifying a swipe gesture (see
  [research.md R2](./research.md#r2-swipe-gesture-detection-fr-002fr-003)); exists only for the
  duration of handling one input event.
- **Game-over state**: `Board::isGameOver()`, computed on demand (see Board rules above).
- **Win state**: `board.hasReached(2048)`, compared against the persisted `winShown` flag to decide
  whether to show the notification this time.
- **Tile color tier**: a pure function `value → one of a fixed small set of tiers` (see
  [research.md R3](./research.md#r3-color-mode-tiletheme-rendering-fr-016-fr-018-user-story-5)),
  used only for choosing which `Color` enum value to draw a tile with; never persisted.
