#include "raylib.h"
#include "render.h"
#include "board.h"
#include "game.h"
#include <algorithm>


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

void Game::handleSetupClick(BoardLayout& layout, Board& board, Vector2 p) {

    if (setupStep == SetupStep::PlaceHouse) {
        int c = cornerAt(layout, p);
        if (c < 0 || board.corners[c].owner != -1) return;
        for (int r : CORNER_LINKS[c].roads) {
            if (r == NONE) continue;
            int other = ROAD_LINKS[r].corners[0] == c ? ROAD_LINKS[r].corners[1] : ROAD_LINKS[r].corners[0];
            if (board.corners[other].owner != NONE) return;   // distance rule
        }
        board.corners[c].owner = currentPlayer;
        board.corners[c].building = BuildingType::Settlement;
        lastHouse = c;
        setupStep = SetupStep::PlaceRoad;
    }
    else { 
        int e = roadAt(layout, p);
        if (e < 0 || board.roads[e].owner != -1) return;
        if (ROAD_LINKS[e].corners[0] != lastHouse && ROAD_LINKS[e].corners[1] != lastHouse) return; // must touch new house
        board.roads[e].owner = currentPlayer;

        setupStep = SetupStep::PlaceHouse;
        setupTurn++;
        currentPlayer = setupTurn < (int)setupPlacingOrder.size() ? setupPlacingOrder[setupTurn] : setupPlacingOrder[2 * (int)setupPlacingOrder.size() - 1 - setupTurn];
        if(setupTurn >= 2 * (int)setupPlacingOrder.size()) {
            phase = Phase::Main;
            currentPlayer = 0;
        }
    }

}

void Game::handleGameClick(BoardLayout& layout, Board& board, Vector2 p, int lastRoll) {
    Player& player = players[currentPlayer];

    // --- corner ---
    if ( int c = cornerAt(layout, p); c != NONE) {
        CornerState& corner = board.corners[c];

        // --- place settlement ---
        if (corner.owner == NONE && corner.building == BuildingType::None) {
            for (int r : CORNER_LINKS[c].roads) {
                if (r == NONE) continue;
                int other = ROAD_LINKS[r].corners[0] == c ? ROAD_LINKS[r].corners[1] : ROAD_LINKS[r].corners[0];
                if (board.corners[other].owner != NONE) return;   // distance rule
            }
            for (int r : CORNER_LINKS[c].roads) {
                if (r == NONE) continue;
                if (board.roads[r].owner == currentPlayer) goto settlementPlacementAllowed;
            }
            return; // must touch player's road

            settlementPlacementAllowed:
            if (player.resources[Resource::Wood] < 1 || player.resources[Resource::Brick] < 1 || player.resources[Resource::Wool] < 1 || player.resources[Resource::Grain] < 1) return;
            player.resources[Resource::Wood]--;
            player.resources[Resource::Brick]--;
            player.resources[Resource::Wool]--;
            player.resources[Resource::Grain]--;
            corner.owner = currentPlayer;
            corner.building = BuildingType::Settlement;
        }

        // --- upgrade to city ---
        else if (corner.owner == currentPlayer && corner.building == BuildingType::Settlement) {
            if (player.resources[Resource::Grain] < 2 || player.resources[Resource::Ore] < 3) return;
            player.resources[Resource::Grain] -= 2;
            player.resources[Resource::Ore] -= 3;
            corner.building = BuildingType::City;
        }
        return;
    }

    // --- road ---
    else if (int r = roadAt(layout, p); r != NONE) {
        RoadState& road = board.roads[r];
        if (road.owner == NONE) {
            if (board.corners[ROAD_LINKS[r].corners[0]].owner != currentPlayer && board.corners[ROAD_LINKS[r].corners[1]].owner != currentPlayer) {
                for(int c : ROAD_LINKS[r].corners) {
                    for (int rr : CORNER_LINKS[c].roads) {
                        if (rr == NONE) continue;
                        if (board.roads[rr].owner == currentPlayer) goto roadPlacementAllowed;
                    }
                }
                return;
            } // must touch player's building

            roadPlacementAllowed:
            if (player.resources[Resource::Wood] < 1 || player.resources[Resource::Brick] < 1) return;
            player.resources[Resource::Wood]--;
            player.resources[Resource::Brick]--;
            road.owner = currentPlayer;
        }
    }

    if (lastRoll == 7 && !robberyActive) {
        if (int t = tileAt(layout, p); t != NONE && t != board.robberTile) {
            robberyActive = true;
            board.robberTile = t;
        }
    }

    if (robberyActive) {
        int c = cornerAt(layout, p);
        if (c != NONE) {
            CornerState& corner = board.corners[c];
            if (corner.owner != NONE && corner.owner != currentPlayer) {
                Player& victim = players[corner.owner];
                if (!victim.resources.empty()) {
                    std::vector<Resource> victimResources;
                    for (const auto& [res, count] : victim.resources) {
                        for (int i = 0; i < count; ++i) {
                            victimResources.push_back(res);
                        }
                    }
                    std::uniform_int_distribution<int> dist(0, victimResources.size() - 1);
                    Resource stolenResource = victimResources[dist(rng)];
                    victim.resources[stolenResource]--;
                    player.resources[stolenResource]++;
                }
                robberyActive = false;
            }
        }
    }
}
