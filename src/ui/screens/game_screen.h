#pragma once
#include "core/board.h"
#include "platform/renderer.h"
#include "ui/screens/screen.h"
#include "ui/widgets.h"

namespace kobo_2048::ui {

// The core game view: 4x4 board, current/best score panel, New Game and
// Settings entry points, swipe-to-move, and the win/game-over dialogs.
class GameScreen : public Screen {
public:
    explicit GameScreen(App& app);

    void draw() override;
    void onTap(Tap tap) override;

private:
    enum class ActiveDialog { None, Win, GameOver };

    void layout();
    void drawBoard();
    void drawCell(Rect rect, uint32_t value);
    void drawScorePanel();
    void handleMove(core::Direction d);
    void showWinDialog();
    void showGameOverDialog();

    Rect boardRect_{};
    Rect titleRect_{};
    Rect scoreRect_{}, bestRect_{};
    Button newGameButton_{}, settingsButton_{}, exitButton_{};

    ActiveDialog activeDialog_ = ActiveDialog::None;
    Dialog dialog_;
    bool gameOver_ = false;  // latched: further swipes ignored until New Game (FR-010)
};

}  // namespace kobo_2048::ui
