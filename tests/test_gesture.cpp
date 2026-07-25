#include "doctest/doctest.h"

#include "ui/gesture.h"

using namespace kobo_2048;
using namespace kobo_2048::ui;
using kobo_2048::core::Direction;

TEST_CASE("a gesture below the deadzone returns nullopt") {
    Tap t{105, 102, 100, 100};  // dx=5, dy=2, threshold=10
    CHECK_FALSE(classifySwipe(t, 10).has_value());
}

TEST_CASE("a gesture exactly at the deadzone threshold is classified, not swallowed") {
    Tap t{110, 100, 100, 100};  // dx=10, dy=0, threshold=10 -> not < threshold
    auto d = classifySwipe(t, 10);
    REQUIRE(d.has_value());
    CHECK(*d == Direction::Right);
}

TEST_CASE("classifySwipe resolves each of the four directions") {
    CHECK(*classifySwipe(Tap{200, 100, 100, 100}, 10) == Direction::Right);
    CHECK(*classifySwipe(Tap{0, 100, 100, 100}, 10) == Direction::Left);
    CHECK(*classifySwipe(Tap{100, 200, 100, 100}, 10) == Direction::Down);
    CHECK(*classifySwipe(Tap{100, 0, 100, 100}, 10) == Direction::Up);
}

TEST_CASE("a diagonal gesture resolves to the axis with the larger absolute delta") {
    CHECK(*classifySwipe(Tap{130, 110, 100, 100}, 10) == Direction::Right);  // dx=30 > dy=10
    CHECK(*classifySwipe(Tap{110, 130, 100, 100}, 10) == Direction::Down);   // dy=30 > dx=10
    CHECK(*classifySwipe(Tap{70, 110, 100, 100}, 10) == Direction::Left);    // dx=-30 dominant
    CHECK(*classifySwipe(Tap{110, 70, 100, 100}, 10) == Direction::Up);      // dy=-30 dominant
}
