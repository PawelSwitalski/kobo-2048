#include "doctest/doctest.h"

#include "core/game_session.h"

using namespace kobo_2048::core;

TEST_CASE("newGame starts with score 0, winShown false, two tiles") {
    Rng rng(3);
    GameSession s = GameSession::newGame(rng);
    CHECK(s.score() == 0);
    CHECK_FALSE(s.winShown());
    int nonZero = 0;
    for (uint32_t v : s.board().cells())
        if (v != 0) ++nonZero;
    CHECK(nonZero == 2);
}

TEST_CASE("move applies the merge and adds score only when the board changes") {
    std::string j =
        R"({"schemaVersion":1,"board":[2,2,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0],"score":0,"winShown":false})";
    GameSession s = GameSession::fromJson(j);
    Rng rng(42);
    MoveResult r = s.move(Direction::Left, rng);
    CHECK(r.changed);
    CHECK(r.scoreGained == 4);
    CHECK(s.score() == 4);
}

TEST_CASE("a no-op move leaves score unchanged and does not spawn") {
    std::string j =
        R"({"schemaVersion":1,"board":[2,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0],"score":10,"winShown":false})";
    GameSession s = GameSession::fromJson(j);
    Rng rng(1);
    MoveResult r = s.move(Direction::Left, rng);  // already at the left edge
    CHECK_FALSE(r.changed);
    CHECK(s.score() == 10);
    int nonZero = 0;
    for (uint32_t v : s.board().cells())
        if (v != 0) ++nonZero;
    CHECK(nonZero == 1);  // still just the one original tile, nothing spawned
}

TEST_CASE("justWon fires exactly once, on the move that first reaches 2048") {
    std::string j =
        R"({"schemaVersion":1,"board":[1024,1024,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0],"score":0,"winShown":false})";
    GameSession s = GameSession::fromJson(j);
    Rng rng(5);
    CHECK_FALSE(s.justWon());
    MoveResult r = s.move(Direction::Left, rng);
    CHECK(r.changed);
    CHECK(s.board().hasReached(2048));
    CHECK(s.justWon());
    CHECK(s.winShown());
}

TEST_CASE("winShown does not re-trigger justWon on a later move") {
    std::string j =
        R"({"schemaVersion":1,"board":[2048,2,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0],"score":2048,"winShown":true})";
    GameSession s = GameSession::fromJson(j);
    Rng rng(1);
    MoveResult r = s.move(Direction::Right, rng);
    CHECK(r.changed);
    CHECK_FALSE(s.justWon());
    CHECK(s.winShown());
}

TEST_CASE("GameSession JSON round-trip is lossless") {
    Rng rng(11);
    GameSession a = GameSession::newGame(rng);
    a.move(Direction::Left, rng);
    GameSession b = GameSession::fromJson(a.toJson());
    CHECK(b.score() == a.score());
    CHECK(b.winShown() == a.winShown());
    CHECK(b.board().cells() == a.board().cells());
}

TEST_CASE("malformed game session files are rejected") {
    CHECK_THROWS_AS(GameSession::fromJson(""), std::exception);
    CHECK_THROWS_AS(GameSession::fromJson("{"), std::exception);
    CHECK_THROWS_AS(GameSession::fromJson("[]"), std::exception);
    CHECK_THROWS_AS(
        GameSession::fromJson(
            R"({"schemaVersion":2,"board":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"score":0,"winShown":false})"),
        std::exception);
    CHECK_THROWS_AS(
        GameSession::fromJson(R"({"schemaVersion":1,"board":[0,0,0],"score":0,"winShown":false})"),
        std::exception);
    CHECK_THROWS_AS(
        GameSession::fromJson(
            R"({"schemaVersion":1,"board":[3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"score":0,"winShown":false})"),
        std::exception);
    CHECK_THROWS_AS(
        GameSession::fromJson(
            R"({"schemaVersion":1,"board":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"score":-1,"winShown":false})"),
        std::exception);
    CHECK_THROWS_AS(
        GameSession::fromJson(
            R"({"schemaVersion":1,"board":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]})"),
        std::exception);
}
