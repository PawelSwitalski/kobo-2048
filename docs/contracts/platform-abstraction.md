# Contract: Platform Abstraction (`src/platform`)

Two interfaces isolate everything device-specific. The UI layer may use **only** these; `core` uses neither.

## Renderer (`renderer.h`)

```cpp
struct DisplayInfo { int width, height, dpi; bool color; };

// Grayscale-first; color is accent-only. ChromeBg/ChromePanel/ChromeButton
// and Tile1..Tile7/TileHigh are the 2048 game's Color-mode theme accents —
// a small closed set, mapped once (RGB) in CanvasRenderer::fillRect /
// SoftCanvas::drawText, the single shared location both backends draw
// through. Black & White mode uses none of these, only Gray.
enum class Color { None, Red, ChromeBg, ChromePanel, ChromeButton,
                   Tile1, Tile2, Tile3, Tile4, Tile5, Tile6, Tile7, TileHigh };

class Renderer {
public:
  virtual DisplayInfo info() const = 0;

  // Drawing (grayscale-first; color is accent-only)
  virtual void fillRect(Rect, Gray shade, Color accent = Color::None) = 0;
  virtual void drawText(Rect, std::string_view, const TextStyle&) = 0;   // sizes in px derived from dpi
  virtual void drawLine(Point, Point, int thicknessPx, Gray) = 0;

  // E-ink refresh discipline
  virtual void flushPartial(Rect) = 0;   // fast, may ghost
  virtual void flushFull() = 0;          // flashing, clears ghosting

  // User-tunable ghosting policy (Settings screen): n <= 0 disables
  // auto-promotion. No-op on backends without a ghosting concept (e.g. the
  // desktop simulator).
  virtual void setGhostingInterval(int n) {}
};
```

**Backend obligations**:
- `kobo/FbinkRenderer`: maps to FBInk; owns waveform choice; honors device rotation; tracks partial-refresh count and MAY promote a `flushPartial` to `flushFull` after N partials (ghosting policy lives here, not in UI code); implements `setGhostingInterval`.
- `sdl/SdlRenderer`: 1:1 window; `flushFull` may simulate the e-ink flash (invert blink) to make refresh behavior visible during development; inherits the no-op `setGhostingInterval` default.
- Both must render all information without `color`; UI code must never encode meaning in `accent` alone.

## TouchInput (`input.h`)

```cpp
// Display coordinates, post-rotation. x/y is the release (up) position;
// startX/startY is the down position (equal to x/y for a stationary tap).
// Screens that only care about taps (button hit-testing) read x/y and
// ignore startX/startY; ui::classifySwipe(Tap, minDistancePx) consumes both
// to resolve a swipe direction — a pure function in `ui`, not `platform`,
// so it stays host-unit-testable.
struct Tap { int x, y, startX, startY; };

class TouchInput {
public:
  // Blocks up to timeoutMs; returns tap, or nothing on timeout.
  // Timeout wakes the app loop for periodic housekeeping (autosave, idle-exit).
  virtual std::optional<Tap> waitForTap(int timeoutMs) = 0;
};
```

**Backend obligations**:
- `kobo/EvdevTouch`: reads `/dev/input/event*` multitouch type-B; translates raw coordinates into the same rotated space `Renderer::info()` reports; collapses a touch-down/up pair into one `Tap`, reporting both the first raw position seen after touch-down (`startX/startY`) and the position at lift (`x/y`) through the same coordinate mapping.
- `sdl/MouseTouch`: records the position on `SDL_MOUSEBUTTONDOWN` as `startX/startY`, reports both points together on the matching `SDL_MOUSEBUTTONUP`.

## App loop contract (`main.cpp`)

- Select backend at compile time (device) or flag (host).
- On any tap: hit-test UI → call app-state mutator → partial-flush the changed rect(s); full-flush on screen transitions.
- At startup, after loading settings: call `renderer.setGhostingInterval(settings.fullRefreshEvery)` once, before entering the main loop.
- On exit action: persist, restore framebuffer state, return 0 so the launcher hands control back to Nickel.
- On SIGTERM/SIGINT: persist and exit cleanly (device sleep/power events).
