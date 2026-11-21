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
    bool showIndices = false;
    int lastRoll = 0;
    game.phase = Phase::Setup;
    game.createSetupPhasePlacingOrder();
    game.currentPlayer = game.setupPlacingOrder[0];

    while (!WindowShouldClose() && game.phase == Phase::Setup) {
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

        // side panel
        int x = 860, y = 30;
        DrawText(TextFormat("Setup turn: %d", game.setupTurn), x, y, 24, RAYWHITE); y += 50;
        for (int p = 0; p < 4; p++) {
            const Player& pl = game.players[p];
            DrawRectangle(x, y + 2, 16, 16, playerColor(p));
            DrawText(pl.name.c_str(), x + 24, y, 20, p == game.currentPlayer ? YELLOW : RAYWHITE);
            y += 26;
        }
        DrawText("D indices", 20, screenH - 30, 18, GRAY);
        EndDrawing();
    }

    lastRoll = game.rollDice();
    game.computeResourceProductions(board, lastRoll);

    while (!WindowShouldClose() && game.phase == Phase::Main) {
        // ---- input ----
        if (IsKeyPressed(KEY_D))   showIndices = !showIndices;
        if (IsKeyPressed(KEY_TAB)) {
            game.robberyActive = false;
            game.currentPlayer = (game.currentPlayer + 1) % 4;
            lastRoll = game.rollDice();
            game.computeResourceProductions(board, lastRoll);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            game.handleGameClick(layout, board, m, lastRoll);
        }

        // ---- draw ----
        BeginDrawing();
        ClearBackground({22, 30, 40, 255});
        drawBoard(board, layout, showIndices);

        // hover highlight
        Vector2 m = GetMousePosition();
        if (int c = cornerAt(layout, m); c != NONE)  DrawCircleLinesV(layout.corners[c], layout.hexSize * 0.25f, YELLOW);
        else if (int r = roadAt(layout, m); r != NONE)
            DrawLineEx(layout.corners[ROAD_LINKS[r].corners[0]], layout.corners[ROAD_LINKS[r].corners[1]], 4, YELLOW);

        // side panel
        int x = 860, y = 30;
        DrawText(TextFormat("Last roll: %d", lastRoll), x, y, 24, RAYWHITE); y += 50;
        for (int p = 0; p < 4; p++) {
            const Player& pl = game.players[p];
            DrawRectangle(x, y + 2, 16, 16, playerColor(p));
            DrawText(pl.name.c_str(), x + 24, y, 20, p == game.currentPlayer ? YELLOW : RAYWHITE);
            y += 26;
            for (int r = 0; r < 5; r++) {
                auto it = pl.resources.find(static_cast<Resource>(r));
                int n = it == pl.resources.end() ? 0 : it->second;
                DrawText(TextFormat("%s: %d", resourceName(static_cast<Resource>(r)), n), x + 24, y, 16, LIGHTGRAY);
                y += 18;
            }
            y += 14;
        }
        DrawText("SPACE roll  TAB next player  D indices", 20, screenH - 30, 18, GRAY);
        EndDrawing();
    }
    CloseWindow();
}
