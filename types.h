#pragma once

// Shared enums live here so board.h, player.h and game.h can all include them
// without including each other (that caused a circular include before).
enum class Resource { Wood, Brick, Wool, Grain, Ore, Desert };

enum class BuildingType { None, Settlement, City, Road };
