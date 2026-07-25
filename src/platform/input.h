#pragma once
#include <optional>

namespace kobo_2048 {

// Display coordinates, post-rotation (same space as Renderer::info()).
// x/y is the release (up) position — existing meaning, unchanged. startX/Y
// is the down position; equals x/y for a stationary tap. Backends fill both;
// screens that only care about taps (button hit-testing) can ignore
// startX/Y, while GameScreen classifies the gesture via ui::classifySwipe.
struct Tap {
    int x = 0, y = 0;
    int startX = 0, startY = 0;
};

// See docs/contracts/platform-abstraction.md.
class TouchInput {
public:
    virtual ~TouchInput() = default;

    // Blocks up to timeoutMs; returns the tap, or nothing on timeout. The
    // timeout wakes the app loop for timer updates and housekeeping.
    virtual std::optional<Tap> waitForTap(int timeoutMs) = 0;
};

}  // namespace kobo_2048
