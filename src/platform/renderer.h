#pragma once
#include <cstdint>
#include <string_view>

namespace kobo_2048 {

struct Point {
    int x = 0, y = 0;
};

struct Rect {
    int x = 0, y = 0, w = 0, h = 0;
    bool contains(Point p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
    // Smallest rect covering both (treats empty rects as identity).
    Rect unite(const Rect& o) const;
};

// Grayscale-first palette (Constitution II). Values are 0=black..255=white;
// backends may quantize to the device's gray levels.
enum class Gray : uint8_t {
    Black = 0x00,
    Dark = 0x55,
    Mid = 0xAA,
    Light = 0xD8,
    Lighter = 0xEE,
    White = 0xFF,
};

// Accent color: optional, meaning must never be encoded in color alone
// (FR-014) — Black & White mode uses none of these, only Gray + numerals.
// ChromeBg/ChromePanel/ChromeButton are Color mode's whole-UI theme
// (FR-016); Tile1..Tile7/TileHigh are a bounded tile-value color-tier set
// (see ui::tileColorTier) so arbitrarily high "keep going" tiles still map
// to a fixed, reviewable palette.
enum class Color : uint8_t {
    None,
    Red,
    ChromeBg,
    ChromePanel,
    ChromeButton,
    Tile1,
    Tile2,
    Tile3,
    Tile4,
    Tile5,
    Tile6,
    Tile7,
    TileHigh,
};

// RGB for each named accent — the single shared table both backends draw
// through (via CanvasRenderer::fillRect / SoftCanvas::drawText), so neither
// needs its own color table (research.md R3). Color::None has no defined
// RGB; callers only consult this when accent != Color::None.
inline void resolveAccentRgb(Color accent, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = g = b = 0;
    switch (accent) {
        case Color::Red:          r = 0xB4; g = 0x20; b = 0x20; break;
        case Color::ChromeBg:     r = 0xFA; g = 0xF8; b = 0xEF; break;  // cream background
        case Color::ChromePanel:  r = 0xBB; g = 0xAD; b = 0xA0; break;  // tan-grey score panel
        case Color::ChromeButton: r = 0x8F; g = 0x7A; b = 0x66; break;  // brown button
        case Color::Tile1:        r = 0xEE; g = 0xE4; b = 0xDA; break;  // 2
        case Color::Tile2:        r = 0xED; g = 0xE0; b = 0xC8; break;  // 4
        case Color::Tile3:        r = 0xF2; g = 0xB1; b = 0x79; break;  // 8
        case Color::Tile4:        r = 0xF5; g = 0x95; b = 0x63; break;  // 16
        case Color::Tile5:        r = 0xF6; g = 0x7C; b = 0x5F; break;  // 32
        case Color::Tile6:        r = 0xF6; g = 0x5E; b = 0x3B; break;  // 64
        case Color::Tile7:        r = 0xED; g = 0xCF; b = 0x72; break;  // 128
        case Color::TileHigh:     r = 0xED; g = 0xC2; b = 0x2E; break;  // 256 and above
        case Color::None: break;
    }
}

struct TextStyle {
    int sizePx = 24;
    bool bold = false;
    Gray shade = Gray::Black;
    Color accent = Color::None;
    enum class Align { Left, Center, Right } align = Align::Center;
};

struct DisplayInfo {
    int width = 0, height = 0, dpi = 0;
    bool color = false;
};

// See docs/contracts/platform-abstraction.md. The UI layer uses only this
// interface; core uses nothing from platform.
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual DisplayInfo info() const = 0;

    virtual void fillRect(Rect r, Gray shade, Color accent = Color::None) = 0;
    virtual void drawText(Rect r, std::string_view text, const TextStyle& style) = 0;
    virtual void drawLine(Point a, Point b, int thicknessPx, Gray shade) = 0;

    // E-ink refresh discipline (FR-016): partial is fast and may ghost; full
    // flashes and clears ghosting. Ghosting policy (auto-promotion after N
    // partials) lives in the device backend, not in UI code.
    virtual void flushPartial(Rect r) = 0;
    virtual void flushFull() = 0;

    // User-tunable ghosting policy (Settings screen): n <= 0 disables
    // auto-promotion. No-op on backends without a ghosting concept (e.g.
    // the desktop simulator).
    virtual void setGhostingInterval(int /*n*/) {}
};

inline Rect Rect::unite(const Rect& o) const {
    if (w <= 0 || h <= 0) return o;
    if (o.w <= 0 || o.h <= 0) return *this;
    int x0 = x < o.x ? x : o.x;
    int y0 = y < o.y ? y : o.y;
    int x1 = (x + w > o.x + o.w) ? x + w : o.x + o.w;
    int y1 = (y + h > o.y + o.h) ? y + h : o.y + o.h;
    return {x0, y0, x1 - x0, y1 - y0};
}

}  // namespace kobo_2048
