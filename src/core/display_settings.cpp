#include "core/display_settings.h"

#include <stdexcept>

#include "nlohmann/json.hpp"

namespace kobo_2048::core {

using nlohmann::json;

namespace {
const char* colorModeToString(ColorMode m) { return m == ColorMode::Color ? "color" : "blackWhite"; }

ColorMode colorModeFromString(const std::string& s) {
    if (s == "color") return ColorMode::Color;
    if (s == "blackWhite") return ColorMode::BlackWhite;
    throw std::runtime_error("invalid display settings: colorMode");
}
}  // namespace

std::string DisplaySettings::toJson() const {
    return json{{"schemaVersion", 1},
                {"colorMode", colorModeToString(colorMode)},
                {"fullRefreshEvery", fullRefreshEvery}}
        .dump();
}

DisplaySettings DisplaySettings::fromJson(const std::string& text) {
    json j = json::parse(text);  // throws on malformed JSON
    if (!j.is_object() || j.value("schemaVersion", 0) != 1)
        throw std::runtime_error("invalid display settings: schemaVersion");

    DisplaySettings s;
    s.colorMode = colorModeFromString(j.at("colorMode").get<std::string>());

    int64_t n = j.at("fullRefreshEvery").get<int64_t>();
    if (n < 0 || n > 100000) throw std::runtime_error("invalid display settings: fullRefreshEvery");
    s.fullRefreshEvery = static_cast<int>(n);

    return s;
}

}  // namespace kobo_2048::core
