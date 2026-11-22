#pragma once
#include <array>
#include <random>
#include <vector>
#include "raylib.h"
#include "board.h"
#include "player.h"

struct BoardLayout;


enum class Phase { Setup, Main };
enum class SetupStep { PlaceHouse, PlaceRoad };


class Game {
public:
    std::array<Player, 4> players;
    std::vector<int> setupPlacingOrder;
    int currentPlayer;
    Phase phase = Phase::Setup;
    SetupStep setupStep = SetupStep::PlaceHouse;
    int setupTurn = 0;               
    int lastHouse = -1; 
    bool robberyActive = false;
    bool robberMoved = false;
    bool tradeActive = false;

    Resource tradeRequestResource = Resource::Desert;
    Resource tradeOfferResource = Resource::Desert;


    int rollDice();
    void computeResourceProductions(const Board& board, int roll);
    void createSetupPhasePlacingOrder();
    void handleSetupClick(BoardLayout& layout, Board& board, Vector2 p);
    void handleGameClick(BoardLayout& layout, Board& board, Vector2 p, int lastRoll);
    void handleTradeClick(Vector2 o, Vector2 p);

private:
    std::mt19937 rng{std::random_device{}()};
};
