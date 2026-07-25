# Contract: Platform Abstraction (`src/platform`) — as modified by this feature

This supersedes `docs/contracts/platform-abstraction.md` for the two changes below; everything
else in that document is unchanged by this feature. Implementation tasks must apply both changes
back to `docs/contracts/platform-abstraction.md` and the real headers so the canonical doc stays
accurate (see [research.md R2, R3](./research.md)).

## Renderer (`renderer.h`) — `Color` enum extended

```cpp
struct DisplayInfo { int width, height, dpi; bool color; };   // unchanged

enum class Color : uint8_t {
    None, Red,                                                     // unchanged (existing)
    ChromeBg, ChromePanel, ChromeButton,                           // new: Color-mode UI chrome
    Tile1, Tile2, Tile3, Tile4, Tile5, Tile6, Tile7, TileHigh,      // new: tile value tiers
};

class Renderer {
public:
  virtual DisplayInfo info() const = 0;
  virtual void fillRect(Rect, Gray shade, Color accent = Color::None) = 0;   // unchanged signature
  virtual void drawText(Rect, std::string_view, const TextStyle&) = 0;       // unchanged
  virtual void drawLine(Point, Point, int thicknessPx, Gray) = 0;            // unchanged
  virtual void flushPartial(Rect) = 0;                                       // unchanged
  virtual void flushFull() = 0;                                              // unchanged
  virtual void setGhostingInterval(int n) {}                                 // unchanged, now used (FR-019)
};
```

**Backend obligations** (delta only):
- The RGB value for every new `Color` member is defined in exactly one place —
  `CanvasRenderer::fillRect` (and the matching case in `SoftCanvas::drawText`'s text-color
  branch) — because both `FbinkRenderer` and `SdlRenderer` draw through the shared `SoftCanvas`.
  Neither backend needs its own color table.
- As before, both backends must render all information without `color`; Black & White mode (this
  feature) proves this by construction — it uses none of the new `Color` values, only `Gray`.
- `FbinkRenderer::setGhostingInterval` already exists (`ghostingPartials_`); this feature is its
  first caller. `SdlRenderer` keeps the inherited no-op default.

## TouchInput (`input.h`) — `Tap` extended with the gesture's start point

```cpp
struct Tap {
    int x = 0, y = 0;            // release position — unchanged meaning, existing consumers unaffected
    int startX = 0, startY = 0;  // new: down position; equals x/y for a stationary tap
};

class TouchInput {
public:
  virtual std::optional<Tap> waitForTap(int timeoutMs) = 0;   // unchanged signature
};
```

**Backend obligations** (delta only):
- `kobo/EvdevTouch`: record the **first** `ABS_MT_POSITION_X/Y` (or `ABS_X/Y`, single-touch
  fallback) observed after touch-down as `startX/startY`, in addition to the already-tracked
  last-seen position reported as `x/y` at lift. Both go through the same coordinate
  mapping (swap/mirror/scale) already applied to `x/y`.
- `sdl/MouseTouch`: record the position on `SDL_MOUSEBUTTONDOWN` as `startX/startY`; report both
  points together on the matching `SDL_MOUSEBUTTONUP`.
- Existing consumers that only read `tap.x/tap.y` (every button-hit-test in `Settings`, `About`,
  etc.) require no changes — a plain tap has `startX == x`, `startY == y`.

## New portable helper (`ui`, not `platform`) — swipe classification

Not part of the platform contract (deliberately — it has no OS dependency and lives in `ui` so it
is host-unit-testable per Constitution III):

```cpp
enum class Direction { Up, Down, Left, Right };

// nullopt if the gesture's largest axis delta is below minDistancePx (a deadzone —
// distinguishes a stationary tap from an intended swipe).
std::optional<Direction> classifySwipe(Tap t, int minDistancePx);
```

## App loop contract (`main.cpp`)

Unchanged from `docs/contracts/platform-abstraction.md`, plus: at startup, after loading
`DisplaySettings`, the app shell calls `renderer.setGhostingInterval(settings.fullRefreshEvery)`
once before entering the main loop (mirrors how `theme`/`paths` are already set up before the
loop starts).
