#pragma once
#include <memory>

#include "core/best_score.h"
#include "core/display_settings.h"
#include "core/game_session.h"
#include "platform/renderer.h"
#include "ui/screens/screen.h"
#include "ui/theme.h"

namespace kobo_2048::ui {

// Semantic surface the screens talk to. Implemented by the app shell in
// main.cpp, which owns backends, the RNG, and persistence wiring.
//
// Mutators persist internally after the change (Constitution V) — screens
// never touch RNG or the filesystem directly, only App's state and actions.
class App {
public:
    virtual ~App() = default;

    virtual Renderer& renderer() = 0;
    virtual const Theme& theme() const = 0;

    virtual const core::GameSession& session() const = 0;
    virtual const core::BestScore& bestScore() const = 0;
    virtual const core::DisplaySettings& settings() const = 0;

    // Slides/merges, spawns a tile if the board changed, updates best score,
    // and persists save.json (+ best.json if it rose) — all in one call.
    virtual core::MoveResult move(core::Direction d) = 0;
    // Resets to a fresh game (grid + current score); best score untouched.
    virtual void newGame() = 0;
    // Applies + persists settings.json; also updates theme().colorMode and
    // calls renderer().setGhostingInterval() as needed.
    virtual void setColorMode(core::ColorMode m) = 0;
    virtual void setFullRefreshEvery(int n) = 0;

    // Navigation. Transitions trigger a full redraw + flushFull (Constitution II).
    virtual void push(std::unique_ptr<Screen> s) = 0;
    virtual void pop() = 0;
    virtual void requestExit() = 0;
};

}  // namespace kobo_2048::ui
