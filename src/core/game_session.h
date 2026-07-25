#pragma once
#include <string>

#include "core/board.h"

namespace kobo_2048::core {

// The single in-progress game: board, running score, and whether the win
// notification has already been shown this game (FR-007/FR-009/FR-012).
class GameSession {
public:
    static GameSession newGame(Rng& rng);

    // Applies one move: slide+merge, then (only if the board changed) adds
    // the merged score and spawns a new tile. Composes Board so the UI layer
    // never manipulates Board directly.
    MoveResult move(Direction d, Rng& rng);

    const Board& board() const { return board_; }
    uint32_t score() const { return score_; }
    bool winShown() const { return winShown_; }
    // True only on the move that first reaches 2048 in this game; resets to
    // false at the start of every move() call.
    bool justWon() const { return justWon_; }

    std::string toJson() const;
    static GameSession fromJson(const std::string& text);  // throws on invalid input

private:
    Board board_;
    uint32_t score_ = 0;
    bool winShown_ = false;
    bool justWon_ = false;
};

}  // namespace kobo_2048::core
