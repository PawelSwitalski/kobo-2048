#include "doctest/doctest.h"

#include <array>

#include "core/board.h"

using namespace kobo_2048::core;

namespace {
std::array<uint32_t, 16> gridFromRow(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return {a, b, c, d, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}
std::array<uint32_t, 16> gridFromCol(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    return {a, 0, 0, 0, b, 0, 0, 0, c, 0, 0, 0, d, 0, 0, 0};
}
}  // namespace

TEST_CASE("slide left merges a colliding pair and compacts") {
    Board b = Board::fromCells(gridFromRow(2, 2, 0, 0));
    MoveResult r = b.applyMove(Direction::Left);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(b.at(0, 0) == 4);
    CHECK(b.at(0, 1) == 0);
}

TEST_CASE("slide right merges toward the right edge") {
    Board b = Board::fromCells(gridFromRow(2, 2, 0, 0));
    MoveResult r = b.applyMove(Direction::Right);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(b.at(0, 3) == 4);
    CHECK(b.at(0, 2) == 0);
}

TEST_CASE("slide up merges toward the top edge") {
    Board b = Board::fromCells(gridFromCol(2, 2, 0, 0));
    MoveResult r = b.applyMove(Direction::Up);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(b.at(0, 0) == 4);
    CHECK(b.at(1, 0) == 0);
}

TEST_CASE("slide down merges toward the bottom edge") {
    Board b = Board::fromCells(gridFromCol(2, 2, 0, 0));
    MoveResult r = b.applyMove(Direction::Down);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(b.at(3, 0) == 4);
    CHECK(b.at(2, 0) == 0);
}

TEST_CASE("three equal tiles merge exactly once, nearest the direction of movement (Left)") {
    // spec User Story 1 Scenario 2
    Board b = Board::fromCells(gridFromRow(2, 2, 2, 0));
    MoveResult r = b.applyMove(Direction::Left);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(b.at(0, 0) == 4);  // nearest pair merged
    CHECK(b.at(0, 1) == 2);  // third tile survives unmerged
    CHECK(b.at(0, 2) == 0);
    CHECK(b.at(0, 3) == 0);
}

TEST_CASE("three equal tiles merge exactly once, nearest the direction of movement (Right)") {
    Board b = Board::fromCells(gridFromRow(2, 2, 2, 0));
    MoveResult r = b.applyMove(Direction::Right);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(b.at(0, 3) == 4);  // nearest-to-right pair merged
    CHECK(b.at(0, 2) == 2);  // farthest tile survives unmerged
    CHECK(b.at(0, 1) == 0);
    CHECK(b.at(0, 0) == 0);
}

TEST_CASE("a merged tile does not merge again within the same move") {
    // four equal tiles in a line: only two merges (two pairs), not a
    // cascading merge into a single tile.
    Board b = Board::fromCells(gridFromRow(2, 2, 2, 2));
    MoveResult r = b.applyMove(Direction::Left);
    CHECK(r.scoreGained == 8);  // two merges of value 4 each
    CHECK(b.at(0, 0) == 4);
    CHECK(b.at(0, 1) == 4);
    CHECK(b.at(0, 2) == 0);
    CHECK(b.at(0, 3) == 0);
}

TEST_CASE("a swipe with no legal move leaves the board unchanged") {
    // Already fully compacted left with no equal-value neighbors: swiping
    // left again must not change anything.
    Board b = Board::fromCells(gridFromRow(4, 2, 0, 0));
    std::array<uint32_t, 16> before = b.cells();
    MoveResult r = b.applyMove(Direction::Left);
    CHECK_FALSE(r.changed);
    CHECK(r.scoreGained == 0);
    CHECK(b.cells() == before);
}

TEST_CASE("spawnTile only targets empty cells") {
    Rng rng(12345);
    std::array<uint32_t, 16> cells{};
    cells.fill(2);
    cells[5] = 0;  // exactly one empty cell
    Board b = Board::fromCells(cells);
    CHECK(b.spawnTile(rng));
    CHECK(b.at(1, 1) != 0);  // index 5 = row1,col1
    CHECK(b.full());
}

TEST_CASE("spawnTile on a full board is a no-op returning false") {
    Rng rng(1);
    std::array<uint32_t, 16> cells{};
    cells.fill(2);
    Board b = Board::fromCells(cells);
    CHECK_FALSE(b.spawnTile(rng));
}

TEST_CASE("spawnTile value distribution is approximately 90% two / 10% four") {
    Rng rng(999);
    int twos = 0, fours = 0;
    const int trials = 2000;
    for (int i = 0; i < trials; ++i) {
        Board b;  // empty board
        b.spawnTile(rng);
        uint32_t v = 0;  // find the single spawned tile
        for (uint32_t c : b.cells())
            if (c != 0) v = c;
        if (v == 2) ++twos;
        else if (v == 4) ++fours;
        else FAIL("unexpected spawned value: ", v);
    }
    double twoFrac = static_cast<double>(twos) / trials;
    CHECK(twoFrac > 0.85);
    CHECK(twoFrac < 0.95);
    CHECK(twos + fours == trials);
}

TEST_CASE("initial board has exactly two tiles") {
    Rng rng(7);
    Board b = Board::initial(rng);
    int nonZero = 0;
    for (uint32_t v : b.cells())
        if (v != 0) ++nonZero;
    CHECK(nonZero == 2);
}

TEST_CASE("hasReached finds an exact tile value") {
    std::array<uint32_t, 16> cells{};
    cells[0] = 2048;
    Board b = Board::fromCells(cells);
    CHECK(b.hasReached(2048));
    CHECK_FALSE(b.hasReached(4096));
}

TEST_CASE("isGameOver: full grid with an available merge is not game over") {
    // Row 0: 2,2,4,8 -- col0/col1 can still merge.
    std::array<uint32_t, 16> cells = {2, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2, 4, 8, 16, 32};
    Board b = Board::fromCells(cells);
    CHECK(b.full());
    CHECK_FALSE(b.isGameOver());
}

TEST_CASE("isGameOver: full grid with no adjacent equal pair is game over") {
    // A checkerboard-ish arrangement with no two orthogonal neighbors equal.
    std::array<uint32_t, 16> cells = {2, 4, 2, 4, 4, 2, 4, 2, 2, 4, 2, 4, 4, 2, 4, 2};
    Board b = Board::fromCells(cells);
    CHECK(b.full());
    CHECK(b.isGameOver());
}

TEST_CASE("isGameOver is false while the grid still has an empty cell") {
    Board b;  // all-empty
    CHECK_FALSE(b.full());
    CHECK_FALSE(b.isGameOver());
}
