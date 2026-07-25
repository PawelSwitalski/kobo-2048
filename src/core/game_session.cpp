#include "core/game_session.h"

#include <stdexcept>

#include "nlohmann/json.hpp"

namespace kobo_2048::core {

using nlohmann::json;

GameSession GameSession::newGame(Rng& rng) {
    GameSession s;
    s.board_ = Board::initial(rng);
    return s;
}

MoveResult GameSession::move(Direction d, Rng& rng) {
    justWon_ = false;
    MoveResult r = board_.applyMove(d);
    if (r.changed) {
        score_ += r.scoreGained;
        board_.spawnTile(rng);
        if (!winShown_ && board_.hasReached(2048)) {
            winShown_ = true;
            justWon_ = true;
        }
    }
    return r;
}

std::string GameSession::toJson() const {
    json arr = json::array();
    for (uint32_t v : board_.cells()) arr.push_back(v);
    return json{{"schemaVersion", 1}, {"board", arr}, {"score", score_}, {"winShown", winShown_}}
        .dump();
}

GameSession GameSession::fromJson(const std::string& text) {
    json j = json::parse(text);  // throws on malformed JSON
    if (!j.is_object() || j.value("schemaVersion", 0) != 1)
        throw std::runtime_error("invalid game session: schemaVersion");

    const json& arr = j.at("board");
    if (!arr.is_array() || arr.size() != static_cast<size_t>(Board::kCellCount))
        throw std::runtime_error("invalid game session: board size");

    std::array<uint32_t, Board::kCellCount> cells{};
    for (size_t i = 0; i < arr.size(); ++i) {
        int64_t v = arr.at(i).get<int64_t>();
        if (v < 0 || v > 0xFFFFFFFFLL || !Board::isValidCellValue(static_cast<uint32_t>(v)))
            throw std::runtime_error("invalid game session: cell value");
        cells[i] = static_cast<uint32_t>(v);
    }

    int64_t score = j.at("score").get<int64_t>();
    if (score < 0 || score > 0xFFFFFFFFLL) throw std::runtime_error("invalid game session: score");

    bool winShown = j.at("winShown").get<bool>();  // throws if missing

    GameSession s;
    s.board_ = Board::fromCells(cells);
    s.score_ = static_cast<uint32_t>(score);
    s.winShown_ = winShown;
    return s;
}

}  // namespace kobo_2048::core
