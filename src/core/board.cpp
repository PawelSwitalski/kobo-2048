#include "core/board.h"

#include <vector>

namespace kobo_2048::core {

namespace {

// Maps (direction, line index i, position-within-line j) to a cell index.
// j = 0 is always the cell closest to the direction of travel, so a plain
// "compact + merge toward the front" pass on the extracted line implements
// slide-and-merge for all four directions.
int lineIndex(Direction d, int i, int j) {
    switch (d) {
        case Direction::Left:
            return i * Board::kSize + j;
        case Direction::Right:
            return i * Board::kSize + (Board::kSize - 1 - j);
        case Direction::Up:
            return j * Board::kSize + i;
        case Direction::Down:
            return (Board::kSize - 1 - j) * Board::kSize + i;
    }
    return 0;
}

struct LineResult {
    bool changed = false;
    uint32_t scoreGained = 0;
};

// Compacts non-zero values to the front, merging equal adjacent values once
// each (a tile born of a merge cannot merge again within the same line/move).
LineResult slideMergeLine(std::array<uint32_t, Board::kSize>& line) {
    std::array<uint32_t, Board::kSize> orig = line;

    std::array<uint32_t, Board::kSize> vals{};
    int n = 0;
    for (uint32_t v : line)
        if (v != 0) vals[static_cast<size_t>(n++)] = v;

    std::array<uint32_t, Board::kSize> merged{};
    int m = 0;
    uint32_t scoreGained = 0;
    for (int i = 0; i < n;) {
        if (i + 1 < n && vals[static_cast<size_t>(i)] == vals[static_cast<size_t>(i + 1)]) {
            uint32_t newVal = vals[static_cast<size_t>(i)] * 2;
            merged[static_cast<size_t>(m++)] = newVal;
            scoreGained += newVal;
            i += 2;
        } else {
            merged[static_cast<size_t>(m++)] = vals[static_cast<size_t>(i)];
            i += 1;
        }
    }

    std::array<uint32_t, Board::kSize> result{};
    for (int i = 0; i < m; ++i) result[static_cast<size_t>(i)] = merged[static_cast<size_t>(i)];

    LineResult r;
    r.changed = result != orig;
    r.scoreGained = scoreGained;
    line = result;
    return r;
}

}  // namespace

Board Board::initial(Rng& rng) {
    Board b;
    b.spawnTile(rng);
    b.spawnTile(rng);
    return b;
}

Board Board::fromCells(const std::array<uint32_t, kCellCount>& cells) {
    Board b;
    b.cells_ = cells;
    return b;
}

MoveResult Board::applyMove(Direction d) {
    MoveResult result;
    for (int i = 0; i < kSize; ++i) {
        std::array<uint32_t, kSize> line{};
        for (int j = 0; j < kSize; ++j) line[static_cast<size_t>(j)] = cells_[static_cast<size_t>(lineIndex(d, i, j))];

        LineResult lr = slideMergeLine(line);
        if (lr.changed) result.changed = true;
        result.scoreGained += lr.scoreGained;

        for (int j = 0; j < kSize; ++j)
            cells_[static_cast<size_t>(lineIndex(d, i, j))] = line[static_cast<size_t>(j)];
    }
    return result;
}

bool Board::spawnTile(Rng& rng) {
    std::vector<int> empties;
    empties.reserve(kCellCount);
    for (int idx = 0; idx < kCellCount; ++idx)
        if (cells_[static_cast<size_t>(idx)] == 0) empties.push_back(idx);
    if (empties.empty()) return false;

    std::uniform_int_distribution<size_t> pickCell(0, empties.size() - 1);
    std::uniform_int_distribution<int> pickOdds(1, 100);
    int idx = empties[pickCell(rng)];
    cells_[static_cast<size_t>(idx)] = (pickOdds(rng) <= 90) ? 2 : 4;
    return true;
}

bool Board::hasReached(uint32_t value) const {
    for (uint32_t v : cells_)
        if (v == value) return true;
    return false;
}

bool Board::full() const {
    for (uint32_t v : cells_)
        if (v == 0) return false;
    return true;
}

bool Board::isGameOver() const {
    if (!full()) return false;
    for (int row = 0; row < kSize; ++row) {
        for (int col = 0; col < kSize; ++col) {
            uint32_t v = at(row, col);
            if (col + 1 < kSize && at(row, col + 1) == v) return false;
            if (row + 1 < kSize && at(row + 1, col) == v) return false;
        }
    }
    return true;
}

}  // namespace kobo_2048::core
