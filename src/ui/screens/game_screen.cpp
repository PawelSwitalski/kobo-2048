#include "ui/screens/game_screen.h"

#include <memory>
#include <string>

#include "ui/app.h"
#include "ui/gesture.h"
#include "ui/screens/settings_screen.h"
#include "ui/tile_theme.h"

namespace kobo_2048::ui {

GameScreen::GameScreen(App& app) : Screen(app) {}

void GameScreen::layout() {
    const Theme& t = app_.theme();
    DisplayInfo d = app_.renderer().info();

    // Top row: title (top-left corner) + Exit button (top-right corner,
    // FR-021), sharing one touch-target-tall row so Exit stays a full-size
    // touch target regardless of the title font's natural height.
    int topRowH = t.touchTargetPx;
    int exitW = t.mm(20.0);
    exitButton_.rect = {d.width - t.pad - exitW, t.pad, exitW, topRowH};
    exitButton_.label = "Exit";
    titleRect_ = {t.pad, t.pad, d.width - 2 * t.pad - exitW - t.gap, topRowH};

    int boxH = t.smallPx + t.textPx + t.mm(1.5);
    int boxY = t.pad + topRowH + t.gap;
    int boxW = (d.width - 2 * t.pad - t.gap) / 2;

    scoreRect_ = {t.pad, boxY, boxW, boxH};
    bestRect_ = {t.pad + boxW + t.gap, boxY, boxW, boxH};

    int buttonsH = t.touchTargetPx;
    int buttonY = d.height - t.pad - buttonsH;

    int boardY = boxY + boxH + t.gap * 2;
    int boardMaxH = buttonY - t.gap - boardY;
    int boardSize = d.width - 2 * t.pad;
    if (boardSize > boardMaxH) boardSize = boardMaxH;
    boardRect_ = {(d.width - boardSize) / 2, boardY, boardSize, boardSize};

    int buttonW = (d.width - 2 * t.pad - t.gap) / 2;
    newGameButton_.rect = {t.pad, buttonY, buttonW, buttonsH};
    newGameButton_.label = "New Game";
    settingsButton_.rect = {t.pad + buttonW + t.gap, buttonY, buttonW, buttonsH};
    settingsButton_.label = "Settings";
}

void GameScreen::draw() {
    layout();
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    DisplayInfo d = r.info();

    Color bgAccent = t.useColor() ? Color::ChromeBg : Color::None;
    r.fillRect({0, 0, d.width, d.height}, Gray::White, bgAccent);

    TextStyle title;
    title.sizePx = t.titlePx;
    title.bold = true;
    title.align = TextStyle::Align::Left;
    r.drawText(titleRect_, "2048", title);

    drawScorePanel();
    drawBoard();

    Color buttonAccent = t.useColor() ? Color::ChromeButton : Color::None;
    newGameButton_.accent = buttonAccent;
    settingsButton_.accent = buttonAccent;
    exitButton_.accent = buttonAccent;
    newGameButton_.draw(r, t);
    settingsButton_.draw(r, t);
    exitButton_.draw(r, t);

    if (activeDialog_ != ActiveDialog::None) dialog_.draw(r, t);
}

void GameScreen::drawScorePanel() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    Color panelAccent = t.useColor() ? Color::ChromePanel : Color::None;

    r.fillRect(scoreRect_, Gray::Light, panelAccent);
    r.fillRect(bestRect_, Gray::Light, panelAccent);

    TextStyle label;
    label.sizePx = t.smallPx;
    TextStyle value;
    value.sizePx = t.textPx;
    value.bold = true;

    r.drawText({scoreRect_.x, scoreRect_.y + t.mm(0.5), scoreRect_.w, t.smallPx}, "SCORE", label);
    r.drawText({scoreRect_.x, scoreRect_.y + t.smallPx, scoreRect_.w, scoreRect_.h - t.smallPx},
               std::to_string(app_.session().score()), value);

    r.drawText({bestRect_.x, bestRect_.y + t.mm(0.5), bestRect_.w, t.smallPx}, "BEST", label);
    r.drawText({bestRect_.x, bestRect_.y + t.smallPx, bestRect_.w, bestRect_.h - t.smallPx},
               std::to_string(app_.bestScore().value()), value);
}

void GameScreen::drawBoard() {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();
    const core::Board& board = app_.session().board();

    Color panelAccent = t.useColor() ? Color::ChromePanel : Color::None;
    r.fillRect(boardRect_, Gray::Light, panelAccent);

    int cellGap = t.gap / 2 > 0 ? t.gap / 2 : 1;
    int cellSize = (boardRect_.w - cellGap * 5) / 4;
    for (int row = 0; row < core::Board::kSize; ++row) {
        for (int col = 0; col < core::Board::kSize; ++col) {
            int x = boardRect_.x + cellGap + col * (cellSize + cellGap);
            int y = boardRect_.y + cellGap + row * (cellSize + cellGap);
            drawCell({x, y, cellSize, cellSize}, board.at(row, col));
        }
    }
}

void GameScreen::drawCell(Rect rect, uint32_t value) {
    Renderer& r = app_.renderer();
    const Theme& t = app_.theme();

    if (value == 0) {
        // Plain White so even the lightest tile tier (Lighter) stays visibly
        // distinct from an empty cell without relying on the number alone.
        r.fillRect(rect, Gray::White);
        return;
    }

    Gray shade = tileGrayShade(value);
    Color accent = t.useColor() ? tileColorTier(value) : Color::None;
    r.fillRect(rect, shade, accent);

    std::string text = std::to_string(value);
    // Scale the font down for long values so they stay fully visible within
    // the tile (edge case: tile values up to 5+ digits, e.g. 65536).
    int sizePx = t.mm(9.0);
    if (text.size() == 4) sizePx = t.mm(7.0);
    else if (text.size() == 5) sizePx = t.mm(5.5);
    else if (text.size() >= 6) sizePx = t.mm(4.0);

    TextStyle st;
    st.sizePx = sizePx;
    st.bold = true;
    st.shade = tileNeedsLightText(value) ? Gray::White : Gray::Black;
    r.drawText(rect, text, st);
}

void GameScreen::handleMove(core::Direction d) {
    core::MoveResult r = app_.move(d);
    if (!r.changed) return;  // FR-006: no redraw for a no-op swipe

    drawScorePanel();
    drawBoard();
    app_.renderer().flushPartial(boardRect_.unite(scoreRect_).unite(bestRect_));

    if (app_.session().justWon()) {
        showWinDialog();
    } else if (app_.session().board().isGameOver()) {
        gameOver_ = true;
        showGameOverDialog();
    }
}

void GameScreen::showWinDialog() {
    activeDialog_ = ActiveDialog::Win;
    dialog_ = Dialog::info("You Win!", "You made a 2048 tile!", "Keep Playing");
    dialog_.layout(app_.theme(), app_.renderer().info());
    dialog_.draw(app_.renderer(), app_.theme());
    app_.renderer().flushFull();
}

void GameScreen::showGameOverDialog() {
    activeDialog_ = ActiveDialog::GameOver;
    dialog_ = Dialog::info("Game Over", "No more moves left.", "New Game");
    dialog_.layout(app_.theme(), app_.renderer().info());
    dialog_.draw(app_.renderer(), app_.theme());
    app_.renderer().flushFull();
}

void GameScreen::onTap(Tap tap) {
    const Theme& t = app_.theme();

    // Checked first, even ahead of the modal-dialog branch below, so Exit
    // stays reachable while a win/game-over dialog is open (spec Edge Case,
    // FR-021). requestExit() persists on the way out via the app shell's
    // existing exit path — no confirmation needed here.
    if (exitButton_.hit(tap)) {
        app_.requestExit();
        return;
    }

    if (activeDialog_ != ActiveDialog::None) {
        if (dialog_.hitButton(tap) >= 0) {
            bool wasWin = activeDialog_ == ActiveDialog::Win;
            bool wasGameOver = activeDialog_ == ActiveDialog::GameOver;
            activeDialog_ = ActiveDialog::None;
            if (wasGameOver) {
                app_.newGame();
                gameOver_ = false;
            }
            draw();
            app_.renderer().flushFull();
            // A move can coincide with both winning and getting stuck; make
            // sure the game-over notice still appears after "Keep Playing".
            if (wasWin && app_.session().board().isGameOver()) {
                gameOver_ = true;
                showGameOverDialog();
            }
        }
        return;
    }

    if (newGameButton_.hit(tap)) {
        app_.newGame();
        gameOver_ = false;
        draw();
        app_.renderer().flushFull();
        return;
    }
    if (settingsButton_.hit(tap)) {
        app_.push(std::make_unique<SettingsScreen>(app_));
        return;
    }

    if (gameOver_) return;  // FR-010: swipes ignored until New Game

    auto dir = classifySwipe(tap, t.mm(8.0));
    if (!dir) return;
    handleMove(*dir);
}

}  // namespace kobo_2048::ui
