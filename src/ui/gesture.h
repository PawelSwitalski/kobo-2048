#pragma once
#include <optional>

#include "core/board.h"
#include "platform/input.h"

namespace kobo_2048::ui {

// Classifies a Tap's start->end delta into a swipe Direction. Pure
// arithmetic, no OS dependency, so it is host-unit-testable (Constitution
// I/III) even though the Tap itself came from platform-specific input.
// Returns nullopt if the gesture's largest axis delta is below
// minDistancePx (the deadzone that distinguishes a stationary tap from an
// intended swipe). On a diagonal gesture, the axis with the larger absolute
// delta wins; horizontal wins an exact tie.
std::optional<core::Direction> classifySwipe(Tap t, int minDistancePx);

}  // namespace kobo_2048::ui
