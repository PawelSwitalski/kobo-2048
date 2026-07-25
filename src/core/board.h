#pragma once
#include <array>
#include <cstdint>
#include <random>

namespace kobo_2048::core {

enum class Direction { Up, Down, Left, Right };

// Shared by Board/GameSession for tile spawning; a plain std::mt19937 is
// enough entropy for this game and is trivially seedable for deterministic
// tests.
using Rng = std::mt19937;

struct MoveResult {
    bool changed = false;
    uint32_t scoreGained = 0;
};

// 4x4 grid, row-major (index = row*4 + col). 0 = empty; otherwise a power of
// two starting at 2. Pure logic, no OS/IO (Constitution I).
class Board {
public:
    static constexpr int kSize = 4;
    static constexpr int kCellCount = kSize * kSize;

    // Two starting tiles (FR-001).
    static Board initial(Rng& rng);

    // Deserialization helper; caller (GameSession::fromJson) validates shape
    // via isValidCellValue before calling this.
    static Board fromCells(const std::array<uint32_t, kCellCount>& cells);

    static bool isValidCellValue(uint32_t v) { return v == 0 || (v >= 2 && (v & (v - 1)) == 0); }

    // Slides every tile as far as possible toward d, merging equal-value
    // collisions once per tile per move (FR-003/FR-004). Does not spawn a
    // tile itself (FR-006) — the caller spawns only when MoveResult.changed.
    MoveResult applyMove(Direction d);

    // Places 2 (90%) or 4 (10%) in a random empty cell (FR-005). Returns
    // false if the board is already full.
    bool spawnTile(Rng& rng);

    bool hasReached(uint32_t value) const;
    bool full() const;
    bool isGameOver() const;  // full() and no orthogonally-adjacent equal pair (FR-010)

    uint32_t at(int row, int col) const { return cells_[static_cast<size_t>(row) * kSize + col]; }
    const std::array<uint32_t, kCellCount>& cells() const { return cells_; }

private:
    std::array<uint32_t, kCellCount> cells_{};
};

}  // namespace kobo_2048::core
