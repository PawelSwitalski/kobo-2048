#pragma once
#include <string>

#include "core/display_settings.h"
#include "platform/renderer.h"

namespace kobo_2048::ui {

// Grayscale-first styles and DPI-relative metrics (Constitution II, FR-015).
// All geometry derives from DisplayInfo; nothing is hardcoded in pixels.
struct Theme {
    int dpi = 300;
    bool color = false;  // panel capability (DisplayInfo.color) — hardware, not player choice

    // Player's chosen display mode (FR-016), set at startup and whenever
    // Settings changes it. Screens consult useColor() to decide whether to
    // pass a Color accent alongside their Gray fallback; Black & White mode
    // never passes one, which is what proves FR-014 by construction.
    core::ColorMode colorMode = core::ColorMode::BlackWhite;
    bool useColor() const { return colorMode == core::ColorMode::Color; }

    int touchTargetPx = 0;  // >= 9 mm (T009): ~106 px @300 dpi, ~75 px @212 dpi
    int pad = 0;            // outer screen margin
    int gap = 0;            // spacing between controls

    int titlePx = 0;   // screen titles
    int textPx = 0;    // buttons, labels
    int smallPx = 0;   // secondary text

    int thinLine = 0;   // thin divider/grid line
    int thickLine = 0;  // emphasized divider/grid line

    std::string fontPath, fontBoldPath;

    int mm(double millimetres) const {
        int px = static_cast<int>(millimetres * dpi / 25.4 + 0.5);
        return px < 1 ? 1 : px;
    }
};

Theme makeTheme(const DisplayInfo& d, const std::string& assetsDir);

}  // namespace kobo_2048::ui
