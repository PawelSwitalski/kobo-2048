#include "ui/gesture.h"

namespace kobo_2048::ui {

std::optional<core::Direction> classifySwipe(Tap t, int minDistancePx) {
    int dx = t.x - t.startX;
    int dy = t.y - t.startY;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;

    if (adx < minDistancePx && ady < minDistancePx) return std::nullopt;

    if (adx >= ady) return dx > 0 ? core::Direction::Right : core::Direction::Left;
    return dy > 0 ? core::Direction::Down : core::Direction::Up;
}

}  // namespace kobo_2048::ui
