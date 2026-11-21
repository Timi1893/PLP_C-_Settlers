#include "game.h"
#include <algorithm>
#include "raylib.h"
#include "render.h"
#include "board.h"


int Game::rollDice() {
    std::uniform_int_distribution<int> d(1, 6);
    return d(rng) + d(rng);
}

void Game::computeResourceProductions(const Board& board, int roll) {
    for (int i = 0; i < NUM_TILES; i++) {
        if (board.tiles[i].number != roll || i == board.robberTile) continue;
        Resource res = board.tiles[i].type;

        for (int cornerIndex : TILE_LINKS[i].corners) {
            const CornerState& corner = board.corners[cornerIndex];
            if (corner.owner == NONE || corner.building == BuildingType::None) continue;

            int amount = (corner.building == BuildingType::City) ? 2 : 1;
            players[corner.owner].resources[res] += amount;
        }
    }
}

void Game::createSetupPhasePlacingOrder() {
    std::vector<int> placingOrder = {0,1,2,3};
    std::shuffle(placingOrder.begin(), placingOrder.end(), rng);

    setupPlacingOrder = placingOrder;
}

static void handleSetupClick(Game& game, BoardLayout& layout, Board& board, Vector2 p) {
    int player = game.setupTurn < (int)game.setupPlacingOrder.size() ? game.setupPlacingOrder[game.setupTurn] : game.setupPlacingOrder[2 * game.setupPlacingOrder.size() - 1 - game.setupTurn];

    if (game.setupStep == SetupStep::PlaceHouse) {
        int c = cornerAt(layout, p);
        if (c < 0 || board.corners[c].owner != -1) return;
        std::array<int, 6> adjacentCorners = TILE_LINKS[CORNER_LINKS[c].tiles[0]].corners;  
        for (int corner : adjacentCorners ) {
            if (corner != c && board.corners[corner].owner != -1) return; // distance rule check
        } 
        board.corners[c].owner = player;
        game.lastHouse = c;
        game.setupStep = SetupStep::PlaceRoad;
    }
    else { 
        int e = roadAt(layout, p);
        if (e < 0 || board.roads[e].owner != -1) return;
        if (ROAD_LINKS[e].corners[0] != game.lastHouse && ROAD_LINKS[e].corners[1] != game.lastHouse) return; // must touch new house
        board.roads[e].owner = player;

        game.setupStep = SetupStep::PlaceHouse;
        game.setupTurn++;
        if(game.setupTurn >= 2 * (int)game.setupPlacingOrder.size() - 1) {
            game.phase = Phase::Main;
        }
    }
}

void handleClick(Game& game, BoardLayout& layout, Board& board, Vector2 p) {
    if (game.phase == Phase::Setup)
        handleSetupClick(game, layout, board, p);
    // else: main game clicks
}
