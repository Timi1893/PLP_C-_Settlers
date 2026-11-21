#pragma once
#include <array>
#include "types.h"

constexpr int NUM_TILES = 19, NUM_CORNERS = 54, NUM_ROADS = 72;
constexpr int NONE = -1;

struct TileLinks { 
    std::array<int, 6> corners; // upper-left, upper-right, right, lower-right, lower-left, left
}; 
struct CornerLinks { 
    std::array<int, 3> roads; // NONE if fewer than 3
    std::array<int, 3> tiles; // NONE if fewer than 3
}; 
struct RoadLinks { 
    std::array<int, 2> corners; 
};

inline constexpr std::array<TileLinks, NUM_TILES> TILE_LINKS = {{
    {{  0,  1,  2, 30, 29, 28 }},   // tile  1
    {{  2,  3,  4, 32, 31, 30 }},   // tile  2
    {{  4,  5,  6,  7, 33, 32 }},   // tile  3
    {{ 33,  7,  8,  9, 35, 34 }},   // tile  4
    {{ 35,  9, 10, 11, 12, 36 }},   // tile  5
    {{ 37, 36, 12, 13, 14, 38 }},   // tile  6
    {{ 39, 38, 14, 15, 16, 17 }},   // tile  7
    {{ 41, 40, 39, 17, 18, 19 }},   // tile  8
    {{ 22, 42, 41, 19, 20, 21 }},   // tile  9
    {{ 53, 44, 43, 42, 22, 23 }},   // tile 10
    {{ 25, 26, 45, 44, 53, 24 }},   // tile 11
    {{ 27, 28, 29, 46, 45, 26 }},   // tile 12
    {{ 29, 30, 31, 48, 47, 46 }},   // tile 13
    {{ 31, 32, 33, 34, 49, 48 }},   // tile 14
    {{ 49, 34, 35, 36, 37, 50 }},   // tile 15
    {{ 51, 50, 37, 38, 39, 40 }},   // tile 16
    {{ 43, 52, 51, 40, 41, 42 }},   // tile 17
    {{ 45, 46, 47, 52, 43, 44 }},   // tile 18
    {{ 47, 48, 49, 50, 51, 52 }},   // tile 19
}};

inline constexpr std::array<CornerLinks, NUM_CORNERS> CORNER_LINKS = {{
    {{  0,  5, NONE }, {  0, NONE, NONE }},   // corner  1
    {{  0,  1, NONE }, {  0, NONE, NONE }},   // corner  2
    {{  1,  2,  6 }, {  0,  1, NONE }},   // corner  3
    {{  6,  7, NONE }, {  1, NONE, NONE }},   // corner  4
    {{  7,  8, 11 }, {  1,  2, NONE }},   // corner  5
    {{ 11, 12, NONE }, {  2, NONE, NONE }},   // corner  6
    {{ 12, 13, NONE }, {  2, NONE, NONE }},   // corner  7
    {{ 13, 14, 16 }, {  2,  3, NONE }},   // corner  8
    {{ 16, 17, NONE }, {  3, NONE, NONE }},   // corner  9
    {{ 17, 18, 21 }, {  3,  4, NONE }},   // corner 10
    {{ 21, 22, NONE }, {  4, NONE, NONE }},   // corner 11
    {{ 22, 23, NONE }, {  4, NONE, NONE }},   // corner 12
    {{ 23, 24, 27 }, {  4,  5, NONE }},   // corner 13
    {{ 27, 28, NONE }, {  5, NONE, NONE }},   // corner 14
    {{ 28, 29, 32 }, {  5,  6, NONE }},   // corner 15
    {{ 32, 33, NONE }, {  6, NONE, NONE }},   // corner 16
    {{ 33, 34, NONE }, {  6, NONE, NONE }},   // corner 17
    {{ 34, 35, 38 }, {  6,  7, NONE }},   // corner 18
    {{ 38, 39, NONE }, {  7, NONE, NONE }},   // corner 19
    {{ 39, 40, 43 }, {  7,  8, NONE }},   // corner 20
    {{ 43, 44, NONE }, {  8, NONE, NONE }},   // corner 21
    {{ 44, 45, NONE }, {  8, NONE, NONE }},   // corner 22
    {{ 41, 45, 49 }, {  8,  9, NONE }},   // corner 23
    {{ 49, 50, NONE }, {  9, NONE, NONE }},   // corner 24
    {{ 54, 55, NONE }, { 10, NONE, NONE }},   // corner 25
    {{ 51, 55, NONE }, { 10, NONE, NONE }},   // corner 26
    {{ 51, 52, 59 }, { 10, 11, NONE }},   // corner 27
    {{ 56, 59, NONE }, { 11, NONE, NONE }},   // corner 28
    {{  4,  5, 56 }, {  0, 11, NONE }},   // corner 29
    {{  3,  4, 57 }, {  0, 11, 12 }},   // corner 30
    {{  2,  3, 10 }, {  0,  1, 12 }},   // corner 31
    {{  9, 10, 60 }, {  1, 12, 13 }},   // corner 32
    {{  8,  9, 15 }, {  1,  2, 13 }},   // corner 33
    {{ 14, 15, 20 }, {  2,  3, 13 }},   // corner 34
    {{ 19, 20, 63 }, {  3, 13, 14 }},   // corner 35
    {{ 18, 19, 25 }, {  3,  4, 14 }},   // corner 36
    {{ 24, 25, 26 }, {  4,  5, 14 }},   // corner 37
    {{ 26, 30, 65 }, {  5, 14, 15 }},   // corner 38
    {{ 29, 30, 31 }, {  5,  6, 15 }},   // corner 39
    {{ 31, 35, 37 }, {  6,  7, 15 }},   // corner 40
    {{ 36, 37, 68 }, {  7, 15, 16 }},   // corner 41
    {{ 36, 40, 42 }, {  7,  8, 16 }},   // corner 42
    {{ 41, 42, 48 }, {  8,  9, 16 }},   // corner 43
    {{ 47, 48, 69 }, {  9, 16, 17 }},   // corner 44
    {{ 46, 47, 53 }, {  9, 10, 17 }},   // corner 45
    {{ 52, 53, 58 }, { 10, 11, 17 }},   // corner 46
    {{ 57, 58, 62 }, { 11, 12, 17 }},   // corner 47
    {{ 61, 62, 71 }, { 12, 17, 18 }},   // corner 48
    {{ 60, 61, 64 }, { 12, 13, 18 }},   // corner 49
    {{ 63, 64, 66 }, { 13, 14, 18 }},   // corner 50
    {{ 65, 66, 67 }, { 14, 15, 18 }},   // corner 51
    {{ 67, 68, 70 }, { 15, 16, 18 }},   // corner 52
    {{ 69, 70, 71 }, { 16, 17, 18 }},   // corner 53
    {{ 46, 50, 54 }, {  9, 10, NONE }},   // corner 54
}};

inline constexpr std::array<RoadLinks, NUM_ROADS> ROAD_LINKS = {{
    {{  0,  1 }},   // road  1: corners 1-2
    {{  1,  2 }},   // road  2: corners 2-3
    {{  2, 30 }},   // road  3: corners 3-31
    {{ 29, 30 }},   // road  4: corners 30-31
    {{ 28, 29 }},   // road  5: corners 29-30
    {{  0, 28 }},   // road  6: corners 1-29
    {{  2,  3 }},   // road  7: corners 3-4
    {{  3,  4 }},   // road  8: corners 4-5
    {{  4, 32 }},   // road  9: corners 5-33
    {{ 31, 32 }},   // road 10: corners 32-33
    {{ 30, 31 }},   // road 11: corners 31-32
    {{  4,  5 }},   // road 12: corners 5-6
    {{  5,  6 }},   // road 13: corners 6-7
    {{  6,  7 }},   // road 14: corners 7-8
    {{  7, 33 }},   // road 15: corners 8-34
    {{ 32, 33 }},   // road 16: corners 33-34
    {{  7,  8 }},   // road 17: corners 8-9
    {{  8,  9 }},   // road 18: corners 9-10
    {{  9, 35 }},   // road 19: corners 10-36
    {{ 34, 35 }},   // road 20: corners 35-36
    {{ 33, 34 }},   // road 21: corners 34-35
    {{  9, 10 }},   // road 22: corners 10-11
    {{ 10, 11 }},   // road 23: corners 11-12
    {{ 11, 12 }},   // road 24: corners 12-13
    {{ 12, 36 }},   // road 25: corners 13-37
    {{ 35, 36 }},   // road 26: corners 36-37
    {{ 36, 37 }},   // road 27: corners 37-38
    {{ 12, 13 }},   // road 28: corners 13-14
    {{ 13, 14 }},   // road 29: corners 14-15
    {{ 14, 38 }},   // road 30: corners 15-39
    {{ 37, 38 }},   // road 31: corners 38-39
    {{ 38, 39 }},   // road 32: corners 39-40
    {{ 14, 15 }},   // road 33: corners 15-16
    {{ 15, 16 }},   // road 34: corners 16-17
    {{ 16, 17 }},   // road 35: corners 17-18
    {{ 17, 39 }},   // road 36: corners 18-40
    {{ 40, 41 }},   // road 37: corners 41-42
    {{ 39, 40 }},   // road 38: corners 40-41
    {{ 17, 18 }},   // road 39: corners 18-19
    {{ 18, 19 }},   // road 40: corners 19-20
    {{ 19, 41 }},   // road 41: corners 20-42
    {{ 22, 42 }},   // road 42: corners 23-43
    {{ 41, 42 }},   // road 43: corners 42-43
    {{ 19, 20 }},   // road 44: corners 20-21
    {{ 20, 21 }},   // road 45: corners 21-22
    {{ 21, 22 }},   // road 46: corners 22-23
    {{ 44, 53 }},   // road 47: corners 45-54
    {{ 43, 44 }},   // road 48: corners 44-45
    {{ 42, 43 }},   // road 49: corners 43-44
    {{ 22, 23 }},   // road 50: corners 23-24
    {{ 23, 53 }},   // road 51: corners 24-54
    {{ 25, 26 }},   // road 52: corners 26-27
    {{ 26, 45 }},   // road 53: corners 27-46
    {{ 44, 45 }},   // road 54: corners 45-46
    {{ 24, 53 }},   // road 55: corners 25-54
    {{ 24, 25 }},   // road 56: corners 25-26
    {{ 27, 28 }},   // road 57: corners 28-29
    {{ 29, 46 }},   // road 58: corners 30-47
    {{ 45, 46 }},   // road 59: corners 46-47
    {{ 26, 27 }},   // road 60: corners 27-28
    {{ 31, 48 }},   // road 61: corners 32-49
    {{ 47, 48 }},   // road 62: corners 48-49
    {{ 46, 47 }},   // road 63: corners 47-48
    {{ 34, 49 }},   // road 64: corners 35-50
    {{ 48, 49 }},   // road 65: corners 49-50
    {{ 37, 50 }},   // road 66: corners 38-51
    {{ 49, 50 }},   // road 67: corners 50-51
    {{ 50, 51 }},   // road 68: corners 51-52
    {{ 40, 51 }},   // road 69: corners 41-52
    {{ 43, 52 }},   // road 70: corners 44-53
    {{ 51, 52 }},   // road 71: corners 52-53
    {{ 47, 52 }},   // road 72: corners 48-53
}};

struct TileState {
    Resource type = Resource::Desert;
    int number = 0;
};
struct CornerState {
    BuildingType building = BuildingType::None;
    int owner = NONE;
};
struct RoadState {
    int owner = NONE;
};


class Board {
    public: 
        std::array<TileState,   NUM_TILES>   tiles;
        std::array<CornerState, NUM_CORNERS> corners;
        std::array<RoadState,   NUM_ROADS>   roads;
        int robberTile = NONE;

        void generateBoardStartingState();
};



// ---- compile-time check: every link must also exist in the other direction ----
constexpr bool contains(const std::array<int, 3>& a, int v) {
    for (int x : a) if (x == v) return true;
    return false;
}

constexpr bool boardIsConsistent() {
    for (int r = 0; r < NUM_ROADS; ++r)                       // road -> corner  implies  corner -> road
        for (int c : ROAD_LINKS[r].corners)
            if (!contains(CORNER_LINKS[c].roads, r)) return false;
    for (int t = 0; t < NUM_TILES; ++t)                       // tile -> corner  implies  corner -> tile
        for (int c : TILE_LINKS[t].corners)
            if (!contains(CORNER_LINKS[c].tiles, t)) return false;
    for (int c = 0; c < NUM_CORNERS; ++c) {                   // every corner has 2-3 roads
        int n = 0;
        for (int r : CORNER_LINKS[c].roads) if (r != NONE) ++n;
        if (n < 2) return false;
    }
    return true;
}
static_assert(boardIsConsistent(), "board links are inconsistent");