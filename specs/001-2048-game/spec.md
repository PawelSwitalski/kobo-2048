# Feature Specification: 2048 Game

**Feature Branch**: `001-2048-game`

**Created**: 2026-07-25

**Status**: Draft

**Input**: User description: "specify this project I already use template for this project. I would like to create `2048` game. Here is description the project what I try to recreate on Kobo https://github.com/gabrielecirulli/2048"

## Clarifications

### Session 2026-07-25

- Q: Should the Color/Black & White tile theme be manually selectable regardless of the device's actual panel capability, or should the app detect the panel and restrict the choice? → A: Auto-detect the panel's color capability and preselect accordingly, but still let the player manually override it, with a warning if Color is chosen on a monochrome-only panel.
- Q: What should the color mode be set to the very first time the game is launched, before the player visits Settings? → A: Whatever the auto-detected value is (Color on a color panel, Black & White on a monochrome panel).
- Q: Does the Color/Black & White choice affect only the tile fills, or the entire UI theme? → A: The entire UI theme — background, score panel, buttons, and tile fills all switch together between the two modes.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Play and merge tiles (Priority: P1)

A player opens the game and sees a grid with two starting number tiles.
They swipe on the touchscreen in a direction; every tile slides as far as
it can that way, tiles of equal value that collide merge into one
double-value tile, a new tile appears in an empty spot, and the score
updates. The player repeats this to grow tiles toward higher values.

**Why this priority**: This is the entire core loop of the game. Without
it there is no game — everything else (win/lose detection, restart,
persistence) only has meaning once a player can make moves and see the
board react correctly.

**Independent Test**: Can be fully tested by starting a new game, swiping
in each of the four directions, and confirming tiles slide, equal tiles
merge exactly once per move, a new tile appears only when the board
actually changed, and the score increases by the value of each merge.
Delivers a playable game on its own.

**Acceptance Scenarios**:

1. **Given** a new game with two tiles on the board, **When** the player
   swipes in a direction where at least one tile can move or merge,
   **Then** all movable tiles slide as far as possible in that direction,
   equal-value tiles that collide merge into one tile of double the
   value, and exactly one new tile (2 or 4) appears in a random empty
   cell.
2. **Given** a row or column containing three equal-value tiles adjacent
   to each other, **When** the player swipes toward them, **Then** only
   one pair merges (the tile closest to the direction of movement),
   leaving one unmerged tile of the original value plus one merged tile.
3. **Given** a board where no tile can move or merge in the swiped
   direction, **When** the player swipes that direction, **Then** the
   board is unchanged, no new tile appears, and no move is counted.
4. **Given** a tile merge occurs, **When** the merge completes, **Then**
   the displayed score increases by the value of the resulting merged
   tile.

---

### User Story 2 - Detect game over and start a new game (Priority: P2)

When the grid fills up and no more moves are possible, the player is
told the game has ended, and can start a fresh game at any time — either
after a game over or whenever they want to abandon the current game.

**Why this priority**: A game that never ends and never lets the player
restart is not a complete, closed experience. Detecting the terminal
state and offering a restart is what turns the core loop into a finished
game session.

**Independent Test**: Can be fully tested by filling the board into a
state with no legal moves and confirming a game-over notice appears, and
separately by triggering "New Game" at any point and confirming the
board and score reset while the best score is retained. Delivers a
complete, replayable game session on its own.

**Acceptance Scenarios**:

1. **Given** the grid is completely full, **When** no two adjacent tiles
   (horizontally or vertically) share the same value, **Then** the
   system displays a game-over notification and further swipes have no
   effect until a new game is started.
2. **Given** the grid is completely full, **When** at least one pair of
   adjacent tiles shares the same value, **Then** the game is not over
   and the player can continue swiping.
3. **Given** any point in a game (in progress, won, or over), **When**
   the player chooses to start a new game, **Then** the grid resets to
   two starting tiles, the current score resets to zero, and the best
   score is unchanged.

---

### User Story 3 - Win the game and keep playing (Priority: P3)

The first time a tile reaches the value 2048, the player is congratulated
with a win notification, and can choose to keep playing on the same
board to reach even higher tile values.

**Why this priority**: Winning is the named goal of the game and the
reason the reference game is called "2048," but the moment-to-moment
gameplay (P1) and a bounded session (P2) both work without it — a build
without the win notification is still a real, playable, closeable game.

**Independent Test**: Can be fully tested by driving a board to produce a
2048 tile and confirming a win notification appears exactly once and
that swiping continues to work normally afterward. Delivers the
"named win" experience on its own.

**Acceptance Scenarios**:

1. **Given** a move creates a tile with the value 2048 for the first time
   in the current game, **When** the merge completes, **Then** the
   system displays a win notification with the option to keep playing.
2. **Given** the player has already seen the win notification in the
   current game, **When** they continue merging tiles beyond 2048,
   **Then** no further win notification is shown and play continues
   without an upper limit on tile value.

---

### User Story 4 - Resume progress after closing the app (Priority: P3)

A player closes the app (or it is interrupted by the device sleeping or
losing power) mid-game. When they reopen the app, their board, current
score, and best score are exactly as they left them, so no progress is
lost.

**Why this priority**: Important for trust in a casual e-reader app that
may be closed at any moment, but the game is fully playable within a
single session without it — this rounds out the experience rather than
enabling it.

**Independent Test**: Can be fully tested by playing a few moves, fully
closing the app, reopening it, and confirming the board, current score,
and best score match what was left. Delivers session-to-session
continuity on its own.

**Acceptance Scenarios**:

1. **Given** a game in progress, **When** the app is closed and reopened,
   **Then** the grid, tile values, and current score are identical to
   the moment before closing.
2. **Given** a best score has been achieved in a previous session,
   **When** a new session starts, **Then** the best score is still
   displayed and is not reset by starting a new game.
3. **Given** the app is interrupted (e.g., device sleep or power loss)
   between moves rather than closed normally, **When** it is reopened,
   **Then** progress is restored up to the last fully completed move,
   with no crash and no corrupted board.

---

### User Story 5 - Customize display settings (Priority: P3)

A player opens Settings and chooses between a Color and a Black & White
display theme, and sets how many screen updates happen before the
display forces a full flashing refresh. The color mode choice changes
the entire look (background, score panel, buttons, and tile fills, not
just tiles); the app preselects a starting choice automatically based on
the device's screen, and warns if Color is picked on a screen that can't
show it.

**Why this priority**: Not required to play a single game, but supports
running the same app across both monochrome and color Kobo panels and
lets the player balance screen refresh flashing against ghosting — a
customization layer on top of the core loop.

**Independent Test**: Can be fully tested by opening Settings, switching
between Color and Black & White and confirming the whole UI (not just
tiles) updates accordingly, confirming the initial value matches the
device's detected panel capability, confirming a warning appears when
Color is chosen on a monochrome panel, and by changing the full-refresh
cadence and confirming forced full refreshes occur at the new cadence.

**Acceptance Scenarios**:

1. **Given** the game is launched for the first time, **When** it
   starts, **Then** the color mode is preset to Color if the device
   panel supports color, or Black & White if it does not.
2. **Given** the player opens Settings, **When** they select the other
   color mode, **Then** the entire UI — background, score panel,
   buttons, and tile fills — immediately switches to that mode's
   appearance.
3. **Given** the player selects Color mode, **When** the device panel
   does not support color, **Then** the system displays a warning that
   colors will not render as intended, and still allows the player to
   proceed with the choice.
4. **Given** the player opens Settings, **When** they choose a
   full-refresh cadence (e.g., every 5, 10, or 25 screen updates, or
   Never), **Then** the system forces a full flashing refresh at that
   cadence going forward, in addition to always flashing on screen
   transitions.

---

### User Story 6 - Exit the game (Priority: P2)

A player taps an Exit control in the top-right corner of the game screen
(with the "2048" title in the top-left) to close the app and return to the
Kobo's normal home screen (Nickel), instead of having to wait for the idle
auto-exit timeout or interrupt the device.

**Why this priority**: The app runs full-screen and pauses the device's
normal home software while active (Constitution IV/e-ink app norms); without
a deliberate way to leave, a player is stuck until an idle timeout fires.
This is core to the app being a well-behaved, temporary, full-screen guest
on the device, on par with starting a new game or reaching game state
milestones — not a cosmetic extra.

**Independent Test**: Can be fully tested by tapping the Exit control from
the game screen at any point (mid-game, after a win, after game over) and
confirming the app closes and control returns to the device's normal home
screen, with the in-progress game and best score unaffected the next time
the app is opened.

**Acceptance Scenarios**:

1. **Given** the game screen is showing, **When** the player taps the Exit
   control in the top-right corner, **Then** the app closes and control
   returns to the device's normal home screen.
2. **Given** a game in progress with unsaved-to-disk moves, **When** the
   player taps Exit, **Then** the current board, score, and best score are
   persisted before the app closes, identical to any other exit path (User
   Story 4).
3. **Given** the player is on the Settings screen, **When** they want to
   exit, **Then** they can return to the game screen (Back) and use the
   Exit control there — Exit does not need to be duplicated on every screen.

---

### Edge Cases

- The player taps Exit while a win or game-over dialog is open: the dialog
  does not block exiting; the game state is persisted and the app closes
  the same as from any other state.
- A swipe in a direction with no legal move: the board must stay
  unchanged, no new tile spawns, and it does not count as a move (User
  Story 1, Scenario 3).
- Three or more equal-value tiles in a line: only one merge per tile per
  move, resolved from the direction of movement, never a chain-merge
  within a single move (User Story 1, Scenario 2).
- The grid is full but a merge is still available: this is not game over
  (User Story 2, Scenario 2).
- The app is interrupted mid-move (device sleep, power loss, forced
  close): on reopening, the game resumes from the last fully completed
  move, never a half-applied or corrupted board (User Story 4, Scenario
  3).
- Persisted save data is missing, unreadable, or corrupted: the system
  starts a fresh new game instead of failing to load.
- A tile's value grows large enough that its number is long (e.g. 4-5
  digits): the tile remains legible and distinguishable from others at
  the device's screen size.
- The player reaches 2048 more than once in a game (not applicable,
  since values only double and never repeat downward) — the win
  notification is shown exactly once per game, on first reaching 2048.
- The device's panel color capability cannot be determined at launch:
  the system defaults to Black & White mode, the choice that always
  renders correctly.
- The player selects Color mode on a monochrome-only panel: a warning is
  shown, but the choice is still honored (colors render as the panel's
  native grayscale mapping of them).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST present a 4x4 grid of cells and start a new
  game with exactly two tiles placed in random empty cells, each
  independently valued 2 (90% chance) or 4 (10% chance).
- **FR-002**: Users MUST be able to move all tiles at once in one of four
  directions (up, down, left, right) via a swipe gesture on the
  touchscreen.
- **FR-003**: When a move is made, every tile MUST slide as far as
  possible in the chosen direction, stopping at the grid edge or upon
  reaching another tile it does not merge with.
- **FR-004**: When two tiles of equal value collide while sliding in the
  same move, they MUST merge into a single tile of double the value; a
  tile that results from a merge MUST NOT merge again within the same
  move.
- **FR-005**: After any move that changes the board, the system MUST add
  exactly one new tile (value 2 with 90% probability, value 4 with 10%
  probability) to a randomly chosen empty cell.
- **FR-006**: A swipe that would not move or merge any tile MUST NOT
  change the board, spawn a new tile, or be counted as a move.
- **FR-007**: The system MUST display a running current score, increased
  by the value of the resulting tile every time a merge occurs.
- **FR-008**: The system MUST track and display a best score, updating it
  whenever the current score exceeds the previously stored best score,
  and MUST retain the best score across app restarts and across starting
  new games.
- **FR-009**: The system MUST detect the first time a tile reaches the
  value 2048 in a game and display a win notification offering the
  player the choice to keep playing.
- **FR-010**: The system MUST detect when the grid is full and no two
  adjacent tiles (horizontally or vertically) share the same value, and
  MUST display a game-over notification at that point.
- **FR-011**: Users MUST be able to start a new game at any time, which
  resets the grid to two starting tiles and the current score to zero
  without altering the stored best score.
- **FR-012**: The system MUST persist the in-progress game (grid state
  and current score) after every completed move, so the same game can be
  resumed exactly after the app is closed and reopened.
- **FR-013**: If persisted game data is missing, unreadable, or invalid,
  the system MUST start a new game rather than failing to load or
  crashing.
- **FR-014**: Tile values MUST be visually distinguishable from one
  another without relying on color alone, and MUST remain fully
  distinguishable when Black & White mode is active.
- **FR-015**: The system MUST be fully playable without a network
  connection.
- **FR-016**: Users MUST be able to choose, from a Settings screen,
  between a Color display mode and a Black & White display mode; the
  choice MUST apply to the entire UI (background, score panel, buttons,
  and tile fills), not tiles alone.
- **FR-017**: On first launch, the system MUST preselect the color mode
  automatically based on the device panel's detected color capability
  (Color if the panel supports color, Black & White otherwise).
- **FR-018**: The system MUST display a warning when the player selects
  Color mode on a device panel that does not support color, while still
  allowing the selection to be made.
- **FR-019**: Users MUST be able to choose, from a Settings screen, how
  many screen updates occur before a full flashing refresh is forced
  (e.g., 5 / 10 / 25 / Never), independent of the full refresh that
  always occurs on screen transitions.
- **FR-020**: The system MUST persist the chosen color mode and
  full-refresh cadence across app restarts.
- **FR-021**: The system MUST provide an Exit control in the top-right
  corner of the game screen that closes the app and returns control to the
  device's normal home screen; persisted state at the moment of exit MUST
  match FR-012's every-move persistence, with no additional data loss.

### Key Entities

- **Game Board**: The 4x4 arrangement of cells that holds the current
  positions of all tiles at a point in time.
- **Tile**: A single numbered piece with a value (a power of 2, starting
  at 2) and a position on the board.
- **Game Session**: The current game's live state — board, current
  score, and whether the win notification has already been shown —
  persisted so it can be resumed later.
- **Best Score**: The highest current score ever reached by the player,
  persisted independently of any single game session.
- **Display Settings**: The player's chosen color mode (Color or Black &
  White) and full-refresh cadence (number of screen updates before a
  forced full refresh, or Never), persisted independently of any single
  game session.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A first-time player produces their first merged tile within
  30 seconds of starting a new game, without instructions.
- **SC-002**: Every swipe that has a legal move produces a visible board
  update; every swipe without one produces no change — verified across
  repeated play with zero incorrect outcomes.
- **SC-003**: After fully closing and reopening the app mid-game, the
  board, current score, and best score match the pre-close state 100% of
  the time.
- **SC-004**: A stored best score is never lost across sessions once
  achieved, including after an abnormal interruption (device sleep or
  power loss), verified across repeated close/reopen cycles.
- **SC-005**: Game-over is correctly identified in 100% of tested
  full-grid, no-legal-move board states, with no false positives while a
  legal move remains.
- **SC-006**: All tile values on screen are correctly distinguished by
  players when the display is rendered in pure grayscale.
- **SC-007**: All tile values on screen are correctly distinguished by
  players in both Color mode and Black & White mode.
- **SC-008**: On first launch, the color mode shown matches the device's
  panel capability (Color on a color panel, Black & White on a
  monochrome panel) with no manual step required, verified across both
  device types.
- **SC-009**: From the game screen, a player can close the app in one tap,
  with the device returned to its normal home screen and no progress lost,
  verified from every reachable game state (fresh game, mid-game, win
  dialog open, game-over dialog open).

## Assumptions

- Grid size is fixed at 4x4 and the named win threshold is a 2048 tile,
  matching the reference game; no difficulty levels or alternate grid
  sizes are in scope for this version.
- Touch swipe gestures are the primary and only required input method;
  on-screen directional buttons are not required for v1.
- After winning, play continues with no upper cap on tile value ("keep
  going"), matching the reference game's behavior.
- No undo feature is included, matching the reference game.
- No continuous or sliding animation is required; tiles update directly
  to their resulting positions and values, consistent with this
  project's e-ink refresh constraints.
- A single local player per device; no accounts, online leaderboards, or
  multiplayer.
- The game is untimed, with no move limit or countdown.
- Starting a new game does not require a confirmation prompt, matching
  the reference game's behavior.
- If the device's color capability cannot be determined, the system
  defaults to Black & White mode.
- The full-refresh cadence setting reuses the same option scale as this
  project's sibling Kobo app (5 / 10 / 25 screen updates, or Never).
- Switching color mode or full-refresh cadence applies immediately,
  without requiring an app restart.
