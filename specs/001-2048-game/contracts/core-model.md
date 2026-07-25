# Contract: Core API (`src/core`)

The portable core exposes these interfaces to the UI layer and tests. No OS calls, no I/O, no
rendering — pure logic. Signatures are normative in shape; exact C++ spelling may vary.

## Board

```cpp
enum class Direction { Up, Down, Left, Right };

struct MoveResult { bool changed; uint32_t scoreGained; };

class Board {
public:
  static Board initial(Rng& rng);              // two starting tiles (FR-001)

  MoveResult applyMove(Direction d);            // slide + merge-once-per-tile (FR-003/FR-004)
  bool spawnTile(Rng& rng);                     // FR-005; returns false if no empty cell
  bool hasReached(uint32_t value) const;        // FR-009 (win check, value = 2048)
  bool isGameOver() const;                      // FR-010; derived, not stored
  bool full() const;

  uint32_t at(int row, int col) const;          // 0 = empty
};
```

**Guarantees**:
- `applyMove` never spawns a tile itself and never mutates `Board` if `MoveResult.changed` is
  false (FR-006) — the caller (`GameSession`) spawns a tile only when `changed` is true.
- A merge chain of 3+ equal tiles in one line produces exactly one merge, nearest the direction of
  movement (spec User Story 1 Scenario 2); the tile it produces cannot merge again within the same
  `applyMove` call.
- `isGameOver()` is `true` iff `full()` and no two orthogonally adjacent cells share a value.

## GameSession

```cpp
class GameSession {
public:
  static GameSession newGame(Rng& rng);         // FR-001/FR-011

  MoveResult move(Direction d, Rng& rng);        // applyMove, then spawnTile iff changed;
                                                  // updates score and winShown
  const Board& board() const;
  uint32_t score() const;
  bool winShown() const;                         // has the win notice already been shown this game?
  bool justWon() const;                           // true only on the move that first reaches 2048

  std::string toJson() const;
  static GameSession fromJson(const std::string& text);   // throws on invalid input (FR-013)
};
```

**Behavioral guarantees**:
- `move()` is the only mutator; it composes `Board::applyMove` + conditional `Board::spawnTile` +
  score/`winShown` bookkeeping, so the UI layer never manipulates `Board` directly (mirrors how
  `kobo-sudoku`'s `Session` is the sole mutator of its board).
- `justWon()` is true exactly once per game — the first `move()` call whose resulting board
  satisfies `hasReached(2048)` while `winShown()` was previously false; `move()` sets `winShown`
  to true at that point.
- Serialization round-trips losslessly (`toJson`/`fromJson`); `fromJson` on malformed input throws
  → caller falls back to a fresh `newGame` (FR-013), the same pattern as `Counter::fromJson`.

## BestScore

```cpp
struct BestScore {
  uint32_t value = 0;
  std::string toJson() const;
  static BestScore fromJson(const std::string& text);   // throws on invalid input
};
```

## DisplaySettings

```cpp
enum class ColorMode { Color, BlackWhite };

struct DisplaySettings {
  ColorMode colorMode;
  int fullRefreshEvery = 12;   // 0 = Never

  std::string toJson() const;
  static DisplaySettings fromJson(const std::string& text);   // throws on invalid input

  static DisplaySettings autoDetect(bool panelSupportsColor);  // FR-017
};
```

## Swipe classification (`ui`, not `core` — no board dependency, but portable/host-testable)

```cpp
std::optional<Direction> classifySwipe(Tap t, int minDistancePx);   // see contracts/platform-abstraction.md
```
