#pragma once
#include "core/display_settings.h"
#include "ui/app.h"
#include "ui/widgets.h"

namespace kobo_2048::ui {

// Color/Black & White theme (FR-016/FR-017/FR-018) and full-refresh cadence
// (FR-019) — both apply immediately and persist (FR-020).
class SettingsScreen : public Screen {
public:
    explicit SettingsScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;

private:
    static std::string refreshLabel(int n);
    static const char* colorModeLabel(core::ColorMode m);

    Rect colorLabelRect_{}, refreshLabelRect_{};
    Button colorBtn_, refreshBtn_, backBtn_;

    bool warningActive_ = false;  // FR-018
    Dialog warningDialog_;
};

}  // namespace kobo_2048::ui
