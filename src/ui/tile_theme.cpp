#include "ui/tile_theme.h"

namespace kobo_2048::ui {

namespace {
constexpr uint32_t kTierValues[7] = {2, 4, 8, 16, 32, 64, 128};
}

int tileTierIndex(uint32_t value) {
    for (int i = 0; i < 7; ++i)
        if (value == kTierValues[i]) return i;
    return 7;  // 256 and above
}

Color tileColorTier(uint32_t value) {
    switch (tileTierIndex(value)) {
        case 0: return Color::Tile1;
        case 1: return Color::Tile2;
        case 2: return Color::Tile3;
        case 3: return Color::Tile4;
        case 4: return Color::Tile5;
        case 5: return Color::Tile6;
        case 6: return Color::Tile7;
        default: return Color::TileHigh;
    }
}

Gray tileGrayShade(uint32_t value) {
    switch (tileTierIndex(value)) {
        case 0: return Gray::Lighter;
        case 1: return Gray::Light;
        case 2: return Gray::Mid;
        case 3: return Gray::Mid;
        case 4: return Gray::Dark;
        case 5: return Gray::Dark;
        default: return Gray::Black;  // tiers 6, 7 (128, 256+)
    }
}

bool tileNeedsLightText(uint32_t value) { return tileTierIndex(value) >= 2; }

}  // namespace kobo_2048::ui
