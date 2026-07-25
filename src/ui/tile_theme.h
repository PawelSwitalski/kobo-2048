#pragma once
#include <cstdint>

#include "platform/renderer.h"

namespace kobo_2048::ui {

// Reduces an arbitrary tile value to one of 8 fixed tiers (0 = value 2, 7 =
// 256 and above), so arbitrarily high "keep going" tiles still map to a
// bounded, reviewable set (research.md R3). Pure function, no rendering.
int tileTierIndex(uint32_t value);

// Color-mode tile fill for this value.
Color tileColorTier(uint32_t value);

// Black & White mode tile fill for this value — Gray only, no Color values,
// which is what proves FR-014 by construction.
Gray tileGrayShade(uint32_t value);

// True if the tile's fill is dark enough that its printed number needs
// light (white) text instead of dark text, in either mode.
bool tileNeedsLightText(uint32_t value);

}  // namespace kobo_2048::ui
