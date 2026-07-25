#pragma once
#include <string>

namespace kobo_2048::core {

enum class ColorMode { Color, BlackWhite };

// Player's chosen color theme and full-refresh cadence (FR-016/FR-019),
// persisted independently of any game session.
struct DisplaySettings {
    ColorMode colorMode = ColorMode::BlackWhite;
    int fullRefreshEvery = 12;  // 0 = "Never"; matches FbinkRenderer's existing default

    // FR-017: preselect Color on a color-capable panel, Black & White otherwise.
    static DisplaySettings autoDetect(bool panelSupportsColor) {
        DisplaySettings s;
        s.colorMode = panelSupportsColor ? ColorMode::Color : ColorMode::BlackWhite;
        return s;
    }

    std::string toJson() const;
    static DisplaySettings fromJson(const std::string& text);  // throws on invalid input
};

}  // namespace kobo_2048::core
