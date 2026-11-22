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
        if (setupTurn >= (int)setupPlacingOrder.size()) {
            for (int t : CORNER_LINKS[c].tiles) {
                if (t == NONE) continue;
                Resource res = board.tiles[t].type;
                if (res != Resource::Desert) players[currentPlayer].resources[res]++;
            }
        }
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

    // --- robber: must be resolved before anything else this turn ---
    if (lastRoll == 7 && !robberMoved) {
        int t = tileAt(layout, p);
        if (t == NONE || t == board.robberTile) return;
        board.robberTile = t;
        robberMoved = true;

        // only enter steal mode if someone on this tile actually has cards
        robberyActive = false;
        for (int c : TILE_LINKS[t].corners) {
            int o = board.corners[c].owner;
            if (o == NONE || o == currentPlayer) continue;
            for (const auto& [res, n] : players[o].resources)
                if (n > 0) robberyActive = true;
        }
        return;
    }

    if (robberyActive) {
        int c = cornerAt(layout, p);
        if (c == NONE) return;
        const auto& tc = TILE_LINKS[board.robberTile].corners;
        if (std::find(tc.begin(), tc.end(), c) == tc.end()) return;   // must be on robber tile
        int o = board.corners[c].owner;
        if (o == NONE || o == currentPlayer) return;

        std::vector<Resource> pool;
        for (const auto& [res, n] : players[o].resources)
            for (int i = 0; i < n; ++i) pool.push_back(res);
        if (pool.empty()) return;                                     // pick someone with cards

        std::uniform_int_distribution<int> dist(0, (int)pool.size() - 1);
        Resource stolen = pool[dist(rng)];
        players[o].resources[stolen]--;
        player.resources[stolen]++;
        robberyActive = false;
        return;
    }

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
}

void Game::handleTradeClick(Vector2 o, Vector2 p) {
    Player& player = players[currentPlayer];
    
    if (tradeRequestResource == Resource::Desert) {
        // Select a resource to request
        if (woodAt(o, p)) {
            tradeRequestResource = Resource::Wood;
        } else if (brickAt(o, p)) {
            tradeRequestResource = Resource::Brick;
        } else if (woolAt(o, p)) {
            tradeRequestResource = Resource::Wool;
        } else if (grainAt(o, p)) {
            tradeRequestResource = Resource::Grain;
        } else if (oreAt(o, p)) {
            tradeRequestResource = Resource::Ore;
        }
    } else {
        // Select a resource to offer
        if (woodAt(o, p)) {
            tradeOfferResource = Resource::Wood;
        } else if (brickAt(o, p)) {
            tradeOfferResource = Resource::Brick;
        } else if (woolAt(o, p)) {
            tradeOfferResource = Resource::Wool;
        } else if (grainAt(o, p)) {
            tradeOfferResource = Resource::Grain;
        } else if (oreAt(o, p)) {       
            tradeOfferResource = Resource::Ore;
        }

        if (player.resources[tradeOfferResource] < 4) {
            tradeOfferResource = Resource::Desert; // Reset the offer
            return;
        }
        player.resources[tradeOfferResource] -= 4;
        player.resources[tradeRequestResource] += 1;
        tradeActive = false;
        return;
    }
}
