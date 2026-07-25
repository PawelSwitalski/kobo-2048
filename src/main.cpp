#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "core/best_score.h"
#include "core/display_settings.h"
#include "core/game_session.h"
#include "persist/paths.h"
#include "persist/store.h"
#include "ui/app.h"
#include "ui/screens/game_screen.h"
#include "ui/theme.h"

#if defined(KOBO_2048_BACKEND_SDL)
#include "platform/sdl/mouse_touch.h"
#include "platform/sdl/sdl_renderer.h"
#elif defined(KOBO_2048_BACKEND_FBINK)
#include "platform/kobo/evdev_touch.h"
#include "platform/kobo/fbink_renderer.h"
#endif

namespace {

volatile std::sig_atomic_t g_signalled = 0;
void onSignal(int) { g_signalled = 1; }  // persist-and-exit (device sleep/power)

struct Options {
    int width = 1264, height = 1680, dpi = 300;  // Kobo Libra Colour geometry
    bool color = true;
    const char* dataDir = nullptr;
    const char* assetsDir = nullptr;
};

Options parseArgs(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        auto next = [&](int& out) { if (i + 1 < argc) out = std::atoi(argv[++i]); };
        if (!std::strcmp(argv[i], "--width")) next(o.width);
        else if (!std::strcmp(argv[i], "--height")) next(o.height);
        else if (!std::strcmp(argv[i], "--dpi")) next(o.dpi);
        else if (!std::strcmp(argv[i], "--gray")) o.color = false;
        else if (!std::strcmp(argv[i], "--data-dir") && i + 1 < argc) o.dataDir = argv[++i];
        else if (!std::strcmp(argv[i], "--assets") && i + 1 < argc) o.assetsDir = argv[++i];
    }
    return o;
}

std::string resolveAssetsDir(const Options& o) {
    if (o.assetsDir) return o.assetsDir;
    if (const char* env = std::getenv("KOBO_2048_ASSETS_DIR"); env && *env) return env;
#if defined(KOBO_2048_BACKEND_FBINK)
    return "/mnt/onboard/.adds/kobo_2048/assets";
#else
    return "dist/.adds/kobo_2048/assets";
#endif
}

class AppImpl : public kobo_2048::ui::App {
public:
    AppImpl(kobo_2048::Renderer& r, kobo_2048::ui::Theme& theme, kobo_2048::persist::Paths paths)
        : renderer_(r),
          theme_(theme),
          paths_(std::move(paths)),
          rng_(std::random_device{}()),
          session_(kobo_2048::core::GameSession::newGame(rng_)) {
        if (auto text = kobo_2048::persist::loadFile(paths_.save)) {
            try {
                session_ = kobo_2048::core::GameSession::fromJson(*text);
            } catch (const std::exception& e) {
                // Corrupt save: log, drop the file, degrade to the fresh game
                // already constructed above (FR-013).
                std::fprintf(stderr, "save.json rejected: %s\n", e.what());
                kobo_2048::persist::removeFile(paths_.save);
            }
        }

        if (auto text = kobo_2048::persist::loadFile(paths_.best)) {
            try {
                best_ = kobo_2048::core::BestScore::fromJson(*text);
            } catch (const std::exception& e) {
                std::fprintf(stderr, "best.json rejected: %s\n", e.what());
                kobo_2048::persist::removeFile(paths_.best);
            }
        }
        best_.update(session_.score());

        if (auto text = kobo_2048::persist::loadFile(paths_.settings)) {
            try {
                settings_ = kobo_2048::core::DisplaySettings::fromJson(*text);
            } catch (const std::exception& e) {
                std::fprintf(stderr, "settings.json rejected: %s\n", e.what());
                kobo_2048::persist::removeFile(paths_.settings);
                settings_ = kobo_2048::core::DisplaySettings::autoDetect(renderer_.info().color);
            }
        } else {
            // First launch (FR-017): preselect from the panel's own capability.
            settings_ = kobo_2048::core::DisplaySettings::autoDetect(renderer_.info().color);
        }
        theme_.colorMode = settings_.colorMode;
        renderer_.setGhostingInterval(settings_.fullRefreshEvery);
    }

    kobo_2048::Renderer& renderer() override { return renderer_; }
    const kobo_2048::ui::Theme& theme() const override { return theme_; }

    const kobo_2048::core::GameSession& session() const override { return session_; }
    const kobo_2048::core::BestScore& bestScore() const override { return best_; }
    const kobo_2048::core::DisplaySettings& settings() const override { return settings_; }

    kobo_2048::core::MoveResult move(kobo_2048::core::Direction d) override {
        kobo_2048::core::MoveResult r = session_.move(d, rng_);
        if (r.changed) {
            kobo_2048::persist::saveFileAtomic(paths_.save, session_.toJson());
            uint32_t before = best_.value();
            best_.update(session_.score());
            if (best_.value() != before)
                kobo_2048::persist::saveFileAtomic(paths_.best, best_.toJson());
        }
        return r;
    }

    void newGame() override {
        session_ = kobo_2048::core::GameSession::newGame(rng_);
        kobo_2048::persist::saveFileAtomic(paths_.save, session_.toJson());
    }

    void setColorMode(kobo_2048::core::ColorMode m) override {
        settings_.colorMode = m;
        theme_.colorMode = m;
        kobo_2048::persist::saveFileAtomic(paths_.settings, settings_.toJson());
    }

    void setFullRefreshEvery(int n) override {
        settings_.fullRefreshEvery = n;
        renderer_.setGhostingInterval(n);
        kobo_2048::persist::saveFileAtomic(paths_.settings, settings_.toJson());
    }

    void push(std::unique_ptr<kobo_2048::ui::Screen> s) override {
        stack_.push_back(std::move(s));
        navDirty_ = true;
    }
    void pop() override {
        if (!stack_.empty()) stack_.pop_back();
        navDirty_ = true;
    }
    void requestExit() override { exitRequested_ = true; }

    // --- app-shell surface (not part of the Screen-facing interface) ---
    kobo_2048::ui::Screen* top() { return stack_.empty() ? nullptr : stack_.back().get(); }
    bool exitRequested() const { return exitRequested_; }
    bool consumeNavDirty() { bool v = navDirty_; navDirty_ = false; return v; }

    // Persist everything on the way out (SIGTERM/normal exit); moves and
    // settings changes already save themselves, so this only matters for
    // whatever changed since the last one (Constitution V).
    void saveOnExit() {
        kobo_2048::persist::saveFileAtomic(paths_.save, session_.toJson());
        kobo_2048::persist::saveFileAtomic(paths_.best, best_.toJson());
        kobo_2048::persist::saveFileAtomic(paths_.settings, settings_.toJson());
    }

private:
    kobo_2048::Renderer& renderer_;
    kobo_2048::ui::Theme& theme_;
    kobo_2048::persist::Paths paths_;
    kobo_2048::core::Rng rng_;

    kobo_2048::core::GameSession session_;
    kobo_2048::core::BestScore best_;
    kobo_2048::core::DisplaySettings settings_;

    std::vector<std::unique_ptr<kobo_2048::ui::Screen>> stack_;
    bool navDirty_ = false;
    bool exitRequested_ = false;
};

}  // namespace

int main(int argc, char** argv) {
    Options opt = parseArgs(argc, argv);
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    std::string assets = resolveAssetsDir(opt);
    kobo_2048::persist::Paths paths = kobo_2048::persist::resolveDataDir(opt.dataDir);

    bool sdlQuit = false, sdlRedraw = false;

#if defined(KOBO_2048_BACKEND_SDL)
    kobo_2048::SdlRenderer renderer;
    kobo_2048::ui::Theme theme = kobo_2048::ui::makeTheme(
        {opt.width, opt.height, opt.dpi, opt.color}, assets);
    if (!renderer.init(opt.width, opt.height, opt.dpi, opt.color, theme.fontPath,
                       theme.fontBoldPath)) {
        std::fprintf(stderr, "renderer init failed (fonts at %s?)\n", assets.c_str());
        return 1;
    }
    kobo_2048::MouseTouch touch(&sdlQuit, &sdlRedraw);
#elif defined(KOBO_2048_BACKEND_FBINK)
    kobo_2048::FbinkRenderer renderer;
    if (!renderer.init(assets)) {
        std::fprintf(stderr, "FBInk init failed\n");
        return 1;
    }
    kobo_2048::ui::Theme theme = kobo_2048::ui::makeTheme(renderer.info(), assets);
    kobo_2048::EvdevTouch touch;
    if (!touch.init(renderer.info())) {
        std::fprintf(stderr, "touch input init failed\n");
        return 1;
    }
#else
    (void)sdlQuit; (void)sdlRedraw;
    std::fprintf(stderr, "built without a backend (KOBO_2048_BACKEND=none)\n");
    return 1;
#endif

#if defined(KOBO_2048_BACKEND_SDL) || defined(KOBO_2048_BACKEND_FBINK)
    AppImpl app(renderer, theme, paths);
    app.push(std::make_unique<kobo_2048::ui::GameScreen>(app));
    app.consumeNavDirty();
    app.top()->draw();
    renderer.flushFull();

    const int kTimeoutMs = 20000;       // wakes the loop for periodic housekeeping
    const int kSleepGapMs = 45000;      // wall-clock gap => device slept
    auto lastSteady = std::chrono::steady_clock::now();
    auto lastWall = std::chrono::system_clock::now();

#if defined(KOBO_2048_BACKEND_FBINK)
    // Nickel is paused for as long as we're in the foreground (start.sh), so
    // it can't run its own inactivity/sleep timer either. Staying frozen
    // indefinitely risks a lower-level watchdog forcing a hard power-off
    // instead of a graceful suspend, so give control back on our own after a
    // stretch of no taps. Override via env var; 0 disables.
    int64_t idleExitMs = 300000;  // 5 min
    if (const char* v = std::getenv("KOBO_2048_IDLE_EXIT_SEC"); v && *v) {
        int sec = std::atoi(v);
        idleExitMs = sec > 0 ? static_cast<int64_t>(sec) * 1000 : 0;
    }
#else
    int64_t idleExitMs = 0;
#endif
    auto lastTapSteady = std::chrono::steady_clock::now();

    while (!app.exitRequested() && !g_signalled && !sdlQuit && app.top()) {
        std::optional<kobo_2048::Tap> tap = touch.waitForTap(kTimeoutMs);

        auto nowSteady = std::chrono::steady_clock::now();
        auto nowWall = std::chrono::system_clock::now();
        int64_t wallMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(nowWall - lastWall).count();
        lastSteady = nowSteady;
        lastWall = nowWall;
        bool slept = wallMs > kTimeoutMs + kSleepGapMs || wallMs < 0;

        kobo_2048::ui::Screen* screen = app.top();

        if (slept || sdlRedraw) {
            // Wake-from-sleep: the sleep screen may cover us; repaint fully.
            sdlRedraw = false;
            screen->draw();
            renderer.flushFull();
        }

        if (tap) {
            lastTapSteady = nowSteady;
            screen->onTap(*tap);
        } else {
            screen->onTick(0);
#if defined(KOBO_2048_BACKEND_FBINK)
            // Any touchscreen activity counts, not just a completed tap (a
            // stray touch, a drag, or sensor noise while resting a finger
            // never reaches screen->onTap otherwise, but the user is still
            // clearly present).
            if (touch.lastActivity() > lastTapSteady) lastTapSteady = touch.lastActivity();
#endif
            int64_t idleMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  nowSteady - lastTapSteady)
                                  .count();
            if (idleExitMs > 0 && idleMs >= idleExitMs) {
                std::fprintf(stderr, "idle-exit: %lld ms since last activity (limit %lld)\n",
                             static_cast<long long>(idleMs), static_cast<long long>(idleExitMs));
                app.requestExit();
            }
        }

        if (app.consumeNavDirty()) {
            if (!app.top()) break;
            app.top()->draw();
            renderer.flushFull();  // screen transition: clean full refresh
        }
    }

    // Never lose progress: persist on every exit path (Constitution V).
    app.saveOnExit();
    return 0;
#endif
}
