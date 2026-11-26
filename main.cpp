#define RAYGUI_IMPLEMENTATION   // in exactly ONE .cpp file
#include "raygui.h"
#include "raylib.h"
#include "board.h"
#include "game.h"
#include "render.h"

// Controls (demo only, no rules/costs enforced yet):
//   left click corner : settlement for current player, click again -> city
//   left click edge   : road for current player
//   left click tile   : move robber
//   SPACE             : roll dice + hand out resources
//   TAB               : next player
//   D                 : toggle index labels (matches board_indexing.png)

int main() {
    const int screenW = 1150, screenH = 820;
    InitWindow(screenW, screenH, "Settlers");
    SetTargetFPS(60);

    Board board;
    board.generateBoardStartingState();

    Game game;
    for (int i = 0; i < 4; i++) { game.players[i].id = i; game.players[i].name = TextFormat("Player %d", i + 1); }

    BoardLayout layout = computeLayout({430, screenH / 2.f}, 72);

    const Vector2 resourceBtnOrigin = {860, 610}; 
    bool showIndices = false;
    int lastRoll = 0;
    game.phase = Phase::Setup;
    game.createSetupPhasePlacingOrder();
    game.currentPlayer = game.setupPlacingOrder[0];

    while (!WindowShouldClose() && game.phase == Phase::Setup) {
        if (IsKeyPressed(KEY_D))   showIndices = !showIndices;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            game.handleSetupClick(layout, board, m);
        }

        BeginDrawing();
        ClearBackground({22, 30, 40, 255});
        drawBoard(board, layout, showIndices);

        // hover highlight
        Vector2 m = GetMousePosition();
        if (int c = cornerAt(layout, m); c != NONE)  DrawCircleLinesV(layout.corners[c], layout.hexSize * 0.25f, YELLOW);
        else if (int r = roadAt(layout, m); r != NONE)
            DrawLineEx(layout.corners[ROAD_LINKS[r].corners[0]], layout.corners[ROAD_LINKS[r].corners[1]], 4, YELLOW);

        drawSetupSideboard(game);
        DrawText("D indices", 20, screenH - 30, 18, GRAY);
        if (game.showError) {
            int btn = -1;
            if (GuiMessageBox({375, 330, 400, 140}, "Error", game.errorMessage.c_str(), "OK", &btn) == RESULT_PRESSED)
                game.showError = false;   // OK (btn = 1) or X (btn = 0)
        }
        EndDrawing();
    }

    lastRoll = game.rollDice();
    game.computeResourceProductions(board, lastRoll);

    while (!WindowShouldClose() && game.phase == Phase::Main) {
        // ---- input ----
        if (IsKeyPressed(KEY_D))   showIndices = !showIndices;
        if (IsKeyPressed(KEY_T)) {
            game.tradeActive = !game.tradeActive;
            game.playerTrade = false;
            game.tradeTargetPlayer = -1;
            game.tradeRequestResource = Resource::None;
            game.tradeOfferResource = Resource::None;
        }
        if (IsKeyPressed(KEY_TAB)) {
            game.robberyActive = false;
            game.robberMoved = false;
            game.tradeActive = false;
            game.playerTrade = false;
            game.tradeTargetPlayer = -1;
            game.tradeRequestResource = Resource::None;
            game.tradeOfferResource = Resource::None;
            game.currentPlayer = (game.currentPlayer + 1) % 4;
            game.computeLongestRoad(board);
            game.computeVictoryPoints(board);
            for (int p = 0; p < 4; p++) {
                if (game.players[p].points >= 10) {
                    game.winningPlayer = p;
                    goto gameOver;
                }
            }
            lastRoll = game.rollDice();
            game.computeResourceProductions(board, lastRoll);
            if (lastRoll == 7) {
                for (int p = 0; p < 4; p++) {
                    Player& pl = game.players[p];
                    int totalCards = 0;
                    for (const auto& [res, n] : pl.resources) totalCards += n;
                    if (totalCards > 7) {
                        for (const auto& [res, n] : pl.resources) {
                            if (n > 0) pl.resources[res] = (pl.resources[res] + 1) / 2;
                        }
                    }
                }
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            if (game.tradeActive) {
                game.handleTradeClick(resourceBtnOrigin, m);
            } else {
                game.handleGameClick(layout, board, m, lastRoll);
            }
        }

        // ---- draw ----
        BeginDrawing();
        ClearBackground({22, 30, 40, 255});
        drawBoard(board, layout, showIndices);
        if (game.tradeActive) {
            drawResourceButtons(resourceBtnOrigin);
        }

        // hover highlight
        Vector2 m = GetMousePosition();
        if (int c = cornerAt(layout, m); c != NONE)  DrawCircleLinesV(layout.corners[c], layout.hexSize * 0.25f, YELLOW);
        else if (int r = roadAt(layout, m); r != NONE)
            DrawLineEx(layout.corners[ROAD_LINKS[r].corners[0]], layout.corners[ROAD_LINKS[r].corners[1]], 4, YELLOW);

        drawGameSideboard(game, lastRoll);
        DrawText("TAB end move  T trade  D indices", 20, screenH - 30, 18, GRAY);
        if (game.tradeActive && game.playerTrade) DrawText("TRADING WITH PLAYER", 860, 680, 24, YELLOW);
        else if (game.tradeActive) DrawText("TRADING", 860, 680, 24, YELLOW);
        if (game.showError) {
            int btn = -1;
            if (GuiMessageBox({375, 330, 400, 140}, "Error", game.errorMessage.c_str(), "OK", &btn) == RESULT_PRESSED)
                game.showError = false;   // OK (btn = 1) or X (btn = 0)
        }
        EndDrawing();
    }

    gameOver:

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({22, 30, 40, 255});
        DrawText(TextFormat("%s wins!", game.players[game.winningPlayer].name.c_str()), 860, 30, 24, YELLOW);
        DrawText("Press ESC to exit", 860, 60, 20, GRAY);
        EndDrawing();
    }

    CloseWindow();
}
