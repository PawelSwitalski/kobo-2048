#include "ui/screens/settings_screen.h"

namespace kobo_2048::ui {

std::string SettingsScreen::refreshLabel(int n) { return n > 0 ? std::to_string(n) : "Never"; }

const char* SettingsScreen::colorModeLabel(core::ColorMode m) {
    return m == core::ColorMode::Color ? "Color" : "Black & White";
}

SettingsScreen::SettingsScreen(App& app) : Screen(app) {
    const Theme& t = app_.theme();
    DisplayInfo d = app_.renderer().info();

    int y = t.mm(10.0) + t.titlePx * 2 + t.mm(10.0);
    int x = t.pad * 3;
    int bw = t.mm(38.0);
    colorLabelRect_ = {x, y, d.width - 2 * x - bw - t.gap, t.touchTargetPx};
    colorBtn_ = {{d.width - x - bw, y, bw, t.touchTargetPx}, ""};

    int y2 = y + t.touchTargetPx + t.gap * 2;
    refreshLabelRect_ = {x, y2, d.width - 2 * x - bw - t.gap, t.touchTargetPx};
    refreshBtn_ = {{d.width - x - bw, y2, bw, t.touchTargetPx}, ""};

    int backW = t.mm(50.0);
    backBtn_ = {{(d.width - backW) / 2, d.height - t.pad - t.touchTargetPx * 5 / 4, backW,
                 t.touchTargetPx * 5 / 4},
                "Back"};
}

void SettingsScreen::draw() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    DisplayInfo d = r.info();

    Color bgAccent = t.useColor() ? Color::ChromeBg : Color::None;
    r.fillRect({0, 0, d.width, d.height}, Gray::White, bgAccent);

    TextStyle title;
    title.sizePx = t.titlePx;
    title.bold = true;
    r.drawText({0, t.mm(10.0), d.width, t.titlePx * 2}, "Settings", title);

    Color buttonAccent = t.useColor() ? Color::ChromeButton : Color::None;

    Label{colorLabelRect_, "Color mode", 0, false, TextStyle::Align::Left}.draw(r, t);
    colorBtn_.label = colorModeLabel(app_.settings().colorMode);
    colorBtn_.toggled = app_.settings().colorMode == core::ColorMode::Color;
    colorBtn_.accent = buttonAccent;
    colorBtn_.draw(r, t);

    Label{refreshLabelRect_, "Screen refresh every", 0, false, TextStyle::Align::Left}.draw(r, t);
    refreshBtn_.label = refreshLabel(app_.settings().fullRefreshEvery);
    refreshBtn_.accent = buttonAccent;
    refreshBtn_.draw(r, t);

    backBtn_.accent = buttonAccent;
    backBtn_.draw(r, t);

    if (warningActive_) warningDialog_.draw(r, t);
}

void SettingsScreen::onTap(Tap tap) {
    if (warningActive_) {
        if (warningDialog_.hitButton(tap) >= 0) {
            warningActive_ = false;
            draw();
            app_.renderer().flushFull();
        }
        return;
    }

    if (colorBtn_.hit(tap)) {
        core::ColorMode next = app_.settings().colorMode == core::ColorMode::Color
                                    ? core::ColorMode::BlackWhite
                                    : core::ColorMode::Color;
        app_.setColorMode(next);
        draw();
        app_.renderer().flushFull();  // whole-UI theme change (FR-016): full redraw

        if (next == core::ColorMode::Color && !app_.renderer().info().color) {
            warningActive_ = true;
            warningDialog_ = Dialog::info(
                "Color Mode", "Colors may not display correctly on this screen.", "OK");
            warningDialog_.layout(app_.theme(), app_.renderer().info());
            warningDialog_.draw(app_.renderer(), app_.theme());
            app_.renderer().flushFull();
        }
        return;
    }

    if (refreshBtn_.hit(tap)) {
        // Cycle 5 -> 10 -> 25 -> Never (0) -> 5 ...
        static const int kOptions[] = {5, 10, 25, 0};
        int cur = app_.settings().fullRefreshEvery;
        size_t idx = 0;
        for (size_t i = 0; i < 4; ++i)
            if (kOptions[i] == cur) idx = i;
        int next = kOptions[(idx + 1) % 4];
        app_.setFullRefreshEvery(next);
        refreshBtn_.label = refreshLabel(next);
        refreshBtn_.draw(app_.renderer(), app_.theme());
        app_.renderer().flushPartial(refreshBtn_.rect);
        return;
    }

    if (backBtn_.hit(tap)) app_.pop();
}

}  // namespace kobo_2048::ui
