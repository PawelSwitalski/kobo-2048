#pragma once
#include <chrono>

#include "platform/input.h"
#include "platform/renderer.h"

namespace kobo_2048 {

// Raw evdev multitouch (type B, with type A/BTN_TOUCH fallback) on
// /dev/input/event*. Collapses one touch-down..up into a single Tap in the
// renderer's coordinate space (docs/contracts/platform-abstraction.md).
//
// Panel-to-display mapping differs per Kobo model; it is controlled by env
// vars so it can be calibrated in the field without a rebuild:
//   KOBO_2048_TOUCH_SWAP_XY=1   swap raw x/y first
//   KOBO_2048_TOUCH_MIRROR_X=1  mirror x after swap
//   KOBO_2048_TOUCH_MIRROR_Y=1  mirror y after swap
//   KOBO_2048_TOUCH_DEBUG=1     log raw+mapped taps to stderr (-> crash.log)
class EvdevTouch : public TouchInput {
public:
    ~EvdevTouch() override;

    bool init(const DisplayInfo& display);
    std::optional<Tap> waitForTap(int timeoutMs) override;

    // Timestamp of the most recent raw input activity, even if it never
    // formed a complete tap (a stray touch, a drag, sensor noise while
    // resting a finger). Idle-exit uses this so any touch counts, not just
    // ones that land as a clean down+up cycle.
    std::chrono::steady_clock::time_point lastActivity() const { return lastActivity_; }

private:
    void mapRaw(int rawX, int rawY, int& outX, int& outY) const;

    int fd_ = -1;
    int rawMinX_ = 0, rawMaxX_ = 0, rawMinY_ = 0, rawMaxY_ = 0;
    int viewW_ = 0, viewH_ = 0;
    bool swapXY_ = false, mirrorX_ = false, mirrorY_ = false, debug_ = false;
    std::chrono::steady_clock::time_point lastActivity_ = std::chrono::steady_clock::now();
};

}  // namespace kobo_2048
