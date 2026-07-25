#include "doctest/doctest.h"

#include "core/display_settings.h"

using namespace kobo_2048::core;

TEST_CASE("autoDetect preselects Color on a color-capable panel") {
    DisplaySettings s = DisplaySettings::autoDetect(true);
    CHECK(s.colorMode == ColorMode::Color);
}

TEST_CASE("autoDetect preselects Black & White on a monochrome panel") {
    DisplaySettings s = DisplaySettings::autoDetect(false);
    CHECK(s.colorMode == ColorMode::BlackWhite);
}

TEST_CASE("DisplaySettings JSON round-trip is lossless") {
    DisplaySettings a;
    a.colorMode = ColorMode::Color;
    a.fullRefreshEvery = 25;
    DisplaySettings b = DisplaySettings::fromJson(a.toJson());
    CHECK(b.colorMode == a.colorMode);
    CHECK(b.fullRefreshEvery == a.fullRefreshEvery);

    a.colorMode = ColorMode::BlackWhite;
    a.fullRefreshEvery = 0;  // "Never"
    DisplaySettings c = DisplaySettings::fromJson(a.toJson());
    CHECK(c.colorMode == ColorMode::BlackWhite);
    CHECK(c.fullRefreshEvery == 0);
}

TEST_CASE("malformed display settings files are rejected") {
    CHECK_THROWS_AS(DisplaySettings::fromJson(""), std::exception);
    CHECK_THROWS_AS(DisplaySettings::fromJson("{"), std::exception);
    CHECK_THROWS_AS(DisplaySettings::fromJson("[]"), std::exception);
    CHECK_THROWS_AS(
        DisplaySettings::fromJson(R"({"schemaVersion":2,"colorMode":"color","fullRefreshEvery":5})"),
        std::exception);
    CHECK_THROWS_AS(
        DisplaySettings::fromJson(R"({"schemaVersion":1,"colorMode":"purple","fullRefreshEvery":5})"),
        std::exception);
    CHECK_THROWS_AS(
        DisplaySettings::fromJson(R"({"schemaVersion":1,"colorMode":"color","fullRefreshEvery":-1})"),
        std::exception);
    CHECK_THROWS_AS(DisplaySettings::fromJson(R"({"schemaVersion":1,"colorMode":"color"})"),
                     std::exception);
}
