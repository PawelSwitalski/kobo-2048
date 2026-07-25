#include "platform/sdl/mouse_touch.h"

#include <SDL.h>

namespace kobo_2048 {

std::optional<Tap> MouseTouch::waitForTap(int timeoutMs) {
    Uint32 deadline = SDL_GetTicks() + static_cast<Uint32>(timeoutMs);
    while (true) {
        Uint32 now = SDL_GetTicks();
        if (now >= deadline) return std::nullopt;
        SDL_Event ev;
        if (!SDL_WaitEventTimeout(&ev, static_cast<int>(deadline - now))) return std::nullopt;
        switch (ev.type) {
            case SDL_QUIT:
                *quit_ = true;
                return std::nullopt;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    haveDown_ = true;
                    downX_ = ev.button.x;
                    downY_ = ev.button.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    Tap t{ev.button.x, ev.button.y, ev.button.x, ev.button.y};
                    if (haveDown_) {
                        t.startX = downX_;
                        t.startY = downY_;
                    }
                    haveDown_ = false;
                    return t;
                }
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_EXPOSED ||
                    ev.window.event == SDL_WINDOWEVENT_RESTORED) {
                    *redraw_ = true;
                    return std::nullopt;
                }
                break;
            default:
                break;
        }
    }
}

}  // namespace kobo_2048
