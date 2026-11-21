#pragma once
#include <array>
#include "raylib.h"
#include "board.h"

// Screen positions of every tile centre and corner, computed once from your index tables.
// Road endpoints are just the positions of its two corners (ROAD_LINKS).
struct BoardLayout {
    float hexSize;                                   // centre-to-corner = edge length
    std::array<Vector2, NUM_TILES>   tileCenters;
    std::array<Vector2, NUM_CORNERS> corners;
};

BoardLayout computeLayout(Vector2 boardCenter, float hexSize);

void drawBoard(const Board& board, const BoardLayout& layout, bool showIndices);

// Mouse picking: return the index under point p, or NONE.
int cornerAt(const BoardLayout& layout, Vector2 p);
int roadAt(const BoardLayout& layout, Vector2 p);
int tileAt(const BoardLayout& layout, Vector2 p);

Color playerColor(int player);
const char* resourceName(Resource r);
