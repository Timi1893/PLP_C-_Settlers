#include "raylib.h"
#include "render.h"
#include "board.h"
#include "game.h"
#include <algorithm>

namespace {
// longest path starting at `corner`, using each road at most once
    int longestFrom(const Board& b, int player, int corner, std::array<bool, NUM_ROADS>& used) {
        int best = 0;
        for (int r : CORNER_LINKS[corner].roads) {
            if (r == NONE || used[r] || b.roads[r].owner != player) continue;
            int next = ROAD_LINKS[r].corners[0] == corner ? ROAD_LINKS[r].corners[1] : ROAD_LINKS[r].corners[0];

            used[r] = true;
            int len = 1;
            int o = b.corners[next].owner;
            if (o == NONE || o == player)                  // opponent building breaks the road
                len += longestFrom(b, player, next, used);
            used[r] = false;                               // backtrack

            best = std::max(best, len);
        }
        return best;
    }
}


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
        if (c < 0 || board.corners[c].owner != -1) {
            showError = true;
            errorMessage = "Invalid corner selection for settlement placement.";
            return;
        };
        for (int r : CORNER_LINKS[c].roads) {
            if (r == NONE) continue;
            int other = ROAD_LINKS[r].corners[0] == c ? ROAD_LINKS[r].corners[1] : ROAD_LINKS[r].corners[0];
            if (board.corners[other].owner != NONE) {
                showError = true;
                errorMessage = "Cannot place settlement here due to distance rule.";
                return;
            };   // distance rule
        }
        board.corners[c].owner = currentPlayer;
        board.corners[c].building = BuildingType::Settlement;
        players[currentPlayer].buildings[BuildingType::Settlement]--;
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
        if (e < 0 || board.roads[e].owner != -1) {
            showError = true;
            errorMessage = "Invalid road selection for road placement.";
            return;
        };
        if (ROAD_LINKS[e].corners[0] != lastHouse && ROAD_LINKS[e].corners[1] != lastHouse) {
            showError = true;
            errorMessage = "Road must touch the newly placed settlement.";
            return;
        }
        board.roads[e].owner = currentPlayer;
        players[currentPlayer].buildings[BuildingType::Road]--;

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
        if (t == NONE || t == board.robberTile) {
            showError = true;
            errorMessage = "Invalid tile selection for robber placement.";
            return;
        }
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
        if (c == NONE) {
            showError = true;
            errorMessage = "Invalid corner selection for robbery.";
            return;
        }
        const auto& tc = TILE_LINKS[board.robberTile].corners;
        if (std::find(tc.begin(), tc.end(), c) == tc.end()) {
            showError = true;
            errorMessage = "Corner must be on the robber's tile.";
            return;
        }
        int o = board.corners[c].owner;
        if (o == NONE || o == currentPlayer) {
            showError = true;
            errorMessage = "Cannot rob from an empty corner or your own corner.";
            return;
        }

        std::vector<Resource> pool;
        for (const auto& [res, n] : players[o].resources)
            for (int i = 0; i < n; ++i) pool.push_back(res);
        if (pool.empty()) {
            showError = true;
            errorMessage = "No cards to rob.";
            return;
        }

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
                if (board.corners[other].owner != NONE) {
                    showError = true;
                    errorMessage = "Cannot place settlement here due to distance rule.";
                    return;
                }
            }
            for (int r : CORNER_LINKS[c].roads) {
                if (r == NONE) continue;
                if (board.roads[r].owner == currentPlayer) goto settlementPlacementAllowed;
            }
            showError = true;
            errorMessage = "Must touch player's road.";
            return; // must touch player's road

            settlementPlacementAllowed:
            if (player.resources[Resource::Wood] < 1 || player.resources[Resource::Brick] < 1 || player.resources[Resource::Wool] < 1 || player.resources[Resource::Grain] < 1) {
                showError = true;
                errorMessage = "Not enough resources to place settlement.";
                return;
            }
            if (player.buildings[BuildingType::Settlement] <= 0) {
                showError = true;
                errorMessage = "No settlements available.";
                return;
            }
            player.resources[Resource::Wood]--;
            player.resources[Resource::Brick]--;
            player.resources[Resource::Wool]--;
            player.resources[Resource::Grain]--;
            corner.owner = currentPlayer;
            corner.building = BuildingType::Settlement;
            player.buildings[BuildingType::Settlement]--;
            if ( int port = portAt(layout, p); port != NONE) {
                player.ports.push_back(board.ports[port]);
            }
        }

        // --- upgrade to city ---
        else if (corner.owner == currentPlayer && corner.building == BuildingType::Settlement) {
            if (player.resources[Resource::Grain] < 2 || player.resources[Resource::Ore] < 3) {
                showError = true;
                errorMessage = "Not enough resources to upgrade to city.";
                return;
            }
            if (player.buildings[BuildingType::City] <= 0) {
                showError = true;
                errorMessage = "No cities available.";
                return;
            }
            player.resources[Resource::Grain] -= 2;
            player.resources[Resource::Ore] -= 3;
            corner.building = BuildingType::City;
            player.buildings[BuildingType::Settlement]++;
            player.buildings[BuildingType::City]--;
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
                showError = true;
                errorMessage = "Road must touch player's building.";
                return;
            } // must touch player's building

            roadPlacementAllowed:
            if (player.resources[Resource::Wood] < 1 || player.resources[Resource::Brick] < 1) {
                showError = true;
                errorMessage = "Not enough resources to place road.";
                return;
            }
            if (player.buildings[BuildingType::Road] <= 0) return;
            player.resources[Resource::Wood]--;
            player.resources[Resource::Brick]--;
            road.owner = currentPlayer;
            player.buildings[BuildingType::Road]--;
        }
    }
}

void Game::handleTradeClick(Vector2 o, Vector2 p) {
    Player& player = players[currentPlayer];
    
    if (tradeRequestResource == Resource::None) {
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
            tradeOfferResource = Resource::None; // Reset the offer
            showError = true;
            errorMessage = "Not enough resources to offer for trade.";
            return;
        }
        auto ports = player.ports;
        if (std::find(ports.begin(), ports.end(), tradeOfferResource) != ports.end()){
            player.resources[tradeOfferResource] -= 2;
        } else if (std::find(ports.begin(), ports.end(), Resource::Universal) != ports.end()){
            player.resources[tradeOfferResource] -= 3;
        } else {player.resources[tradeOfferResource] -= 4;}
        player.resources[tradeRequestResource] += 1;
        tradeActive = false;
        return;
    }
}

void Game::computeVictoryPoints(Board& board) {
    for (Player& player : players) {
        player.points = 0;
        for (const CornerState& corner : board.corners) {
            if (corner.owner == player.id) {
                if (corner.building == BuildingType::Settlement) {
                    player.points += 1;
                } else if (corner.building == BuildingType::City) {
                    player.points += 2;
                }
            }
        }
        if (player.id == playerWithLongestRoad) {
            player.points += 2;
        }
    }
}

void Game::computeLongestRoad(Board& board) {
    std::array<int, 4> length{};
    for (int p = 0; p < 4; ++p) {
        std::array<bool, NUM_ROADS> used{};
        for (int c = 0; c < NUM_CORNERS; ++c)
            length[p] = std::max(length[p], longestFrom(board, p, c, used));
    }

    // holder loses it if their road got cut below 5
    if (playerWithLongestRoad != NONE && length[playerWithLongestRoad] < 5)
        playerWithLongestRoad = NONE;

    int best = playerWithLongestRoad == NONE ? 4 : length[playerWithLongestRoad];
    for (int p = 0; p < 4; ++p)
        if (length[p] > best) { best = length[p]; playerWithLongestRoad = p; }
}