#include "board.h"
#include <random>
#include <vector>
#include <algorithm>

static std::mt19937 rng(std::random_device{}());

void Board::generateBoardStartingState() {
    std::vector<int> tile_numbers = {2,3,3,4,4,5,5,6,6,7,8,8,9,9,10,10,11,11,12};
    std::vector<Resource> tile_resources = {
        Resource::Wood, Resource::Wood, Resource::Wood, Resource::Wood,
        Resource::Brick, Resource::Brick, Resource::Brick,
        Resource::Wool, Resource::Wool, Resource::Wool, Resource::Wool,
        Resource::Grain, Resource::Grain, Resource::Grain, Resource::Grain,
        Resource::Ore, Resource::Ore, Resource::Ore };

    for (int i = 0; i < NUM_TILES; i++) {
        std::uniform_int_distribution<int> selectNumber(0, static_cast<int>(tile_numbers.size()) - 1);
        int number_index = selectNumber(rng);
        int number = tile_numbers[number_index];             
        tile_numbers.erase(tile_numbers.begin() + number_index);

        if (number == 7) {                                     
            tiles[i].type = Resource::Desert;
            tiles[i].number = 0;
            robberTile = i;                                
        } else {
            std::uniform_int_distribution<int> selectType(0, static_cast<int>(tile_resources.size()) - 1);
            int type_index = selectType(rng);
            tiles[i].type = tile_resources[type_index];
            tiles[i].number = number;
            tile_resources.erase(tile_resources.begin() + type_index);
        }
    }

    std::array<Resource, NUM_PORTS> portTypes = {
    Resource::Universal, Resource::Universal, Resource::Universal, Resource::Universal,
    Resource::Wood, Resource::Brick, Resource::Wool, Resource::Grain, Resource::Ore };
    std::shuffle(portTypes.begin(), portTypes.end(), rng);
    ports = portTypes;
}

