#include "doctest/doctest.h"

#include "core/best_score.h"

using namespace kobo_2048::core;

TEST_CASE("best score starts at zero") {
    BestScore b;
    CHECK(b.value() == 0);
}

TEST_CASE("best score only ever increases") {
    BestScore b;
    b.update(100);
    CHECK(b.value() == 100);
    b.update(50);
    CHECK(b.value() == 100);  // lower incoming score: unchanged
    b.update(100);
    CHECK(b.value() == 100);  // equal incoming score: unchanged
    b.update(250);
    CHECK(b.value() == 250);  // higher incoming score: updates
}

TEST_CASE("best score JSON round-trip is lossless") {
    BestScore a;
    a.update(4242);
    BestScore b = BestScore::fromJson(a.toJson());
    CHECK(b.value() == a.value());
}

TEST_CASE("malformed best score files are rejected") {
    CHECK_THROWS_AS(BestScore::fromJson(""), std::exception);
    CHECK_THROWS_AS(BestScore::fromJson("{"), std::exception);
    CHECK_THROWS_AS(BestScore::fromJson("[]"), std::exception);
    CHECK_THROWS_AS(BestScore::fromJson(R"({"schemaVersion":2,"value":1})"), std::exception);
    CHECK_THROWS_AS(BestScore::fromJson(R"({"schemaVersion":1,"value":-1})"), std::exception);
}
