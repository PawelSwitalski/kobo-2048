#pragma once
#include "platform/input.h"

namespace kobo_2048 {

// Mouse click -> Tap. Window-close sets *quitFlag; window-exposed sets
// *redrawFlag so the app shell can repaint.
class MouseTouch : public TouchInput {
public:
    MouseTouch(bool* quitFlag, bool* redrawFlag) : quit_(quitFlag), redraw_(redrawFlag) {}

    std::optional<Tap> waitForTap(int timeoutMs) override;

private:
    bool* quit_;
    bool* redraw_;
    bool haveDown_ = false;
    int downX_ = 0, downY_ = 0;
};

}  // namespace kobo_2048
