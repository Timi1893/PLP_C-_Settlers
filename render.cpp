#include "render.h"
#include "raymath.h"
#include <string>

namespace {

constexpr float H = 0.8660254f;   // sqrt(3)/2

// Tile centres on a simple grid, same numbering as board_indexing.png:
// x step = 1.5 * hexSize, y step = H * hexSize (half a hex height).
constexpr int TILE_GRID[NUM_TILES][2] = {
    { 0,-4}, { 1,-3}, { 2,-2}, { 2, 0}, { 2, 2}, { 1, 3},   // outer ring, tile 1 at the top, clockwise
    { 0, 4}, {-1, 3}, {-2, 2}, {-2, 0}, {-2,-2}, {-1,-3},
    { 0,-2}, { 1,-1}, { 1, 1}, { 0, 2}, {-1, 1}, {-1,-1},   // inner ring
    { 0, 0},                                                // centre
};

// Corner offsets (in units of hexSize) in the same order as TileLinks::corners:
// upper-left, upper-right, right, lower-right, lower-left, left.  (y grows downward in raylib)
constexpr Vector2 CORNER_OFFSET[6] = {
    {-0.5f, -H}, {0.5f, -H}, {1.f, 0.f}, {0.5f, H}, {-0.5f, H}, {-1.f, 0.f},
};

Color resourceColor(Resource r) {
    switch (r) {
        case Resource::Wood:  return {34, 110, 52, 255};
        case Resource::Brick: return {178, 82, 45, 255};
        case Resource::Wool:  return {150, 205, 90, 255};
        case Resource::Grain: return {232, 196, 72, 255};
        case Resource::Ore:   return {128, 130, 140, 255};
        default:              return {218, 198, 150, 255};   // Desert
    }
}

constexpr float BTN_SIZE = 44, BTN_GAP = 8;

Rectangle resourceButtonRect(Vector2 origin, Resource r) {
    return { origin.x + static_cast<int>(r) * (BTN_SIZE + BTN_GAP), origin.y, BTN_SIZE, BTN_SIZE };
}

void drawCenteredText(const char* text, Vector2 pos, int fontSize, Color color) {
    int w = MeasureText(text, fontSize);
    DrawText(text, (int)(pos.x - w / 2.f), (int)(pos.y - fontSize / 2.f), fontSize, color);
}

} // namespace

BoardLayout computeLayout(Vector2 boardCenter, float hexSize) {
    BoardLayout layout{};
    layout.hexSize = hexSize;

    for (int t = 0; t < NUM_TILES; t++) {
        Vector2 c = { boardCenter.x + TILE_GRID[t][0] * 1.5f * hexSize,
                      boardCenter.y + TILE_GRID[t][1] * H    * hexSize };
        layout.tileCenters[t] = c;

        // A shared corner gets written by up to 3 tiles, always with the same position.
        for (int k = 0; k < 6; k++) {
            int corner = TILE_LINKS[t].corners[k];
            layout.corners[corner] = { c.x + CORNER_OFFSET[k].x * hexSize,
                                       c.y + CORNER_OFFSET[k].y * hexSize };
        }
    }

    // Ports: push the edge midpoint outward (away from its land tile) into the water
    for (int i = 0; i < NUM_PORTS; i++) {
        Vector2 a = layout.corners[PORT_LINKS[i].corners[0]];
        Vector2 b = layout.corners[PORT_LINKS[i].corners[1]];
        Vector2 mid = Vector2Lerp(a, b, 0.5f);
        Vector2 out = Vector2Normalize(Vector2Subtract(mid, layout.tileCenters[PORT_LINKS[i].tile]));
        layout.ports[i] = Vector2Add(mid, Vector2Scale(out, hexSize * 0.6f));
    }
    return layout;
}

void drawBoard(const Board& board, const BoardLayout& L, bool showIndices) {
    const float s = L.hexSize;

    // Sea background: a big pointy-top hexagon behind the board
    DrawPoly(L.tileCenters[18], 6, s * 5.3f, 30, {40, 105, 170, 255});

    // Tiles
    for (int t = 0; t < NUM_TILES; t++) {
        Vector2 c = L.tileCenters[t];
        DrawPoly(c, 6, s, 0, resourceColor(board.tiles[t].type));           // rotation 0 = flat-top
        DrawPolyLinesEx(c, 6, s, 0, 3, {60, 45, 30, 255});

        int n = board.tiles[t].number;
        if (n > 0) {
            DrawCircleV(c, s * 0.32f, {245, 232, 200, 255});
            Color numColor = (n == 6 || n == 8) ? Color{190, 30, 30, 255} : BLACK;
            drawCenteredText(std::to_string(n).c_str(), c, (int)(s * 0.36f), numColor);
        }
        if (t == board.robberTile)
            DrawCircleV({c.x - s * 0.45f, c.y}, s * 0.16f, {40, 40, 40, 230});
    }

    // Ports
    for (int i = 0; i < NUM_PORTS; i++) {
        Vector2 pc = L.ports[i];
        for (int c : PORT_LINKS[i].corners)                                    // piers to both corners
            DrawLineEx(L.corners[c], pc, s * 0.07f, {120, 85, 50, 255});

        Resource r = board.ports[i];
        bool generic = (r == Resource::Universal);
        DrawCircleV(pc, s * 0.28f, generic ? Color{245, 232, 200, 255} : resourceColor(r));
        DrawCircleLinesV(pc, s * 0.28f, {60, 45, 30, 255});
        drawCenteredText(generic ? "3:1" : "2:1", pc, (int)(s * 0.22f), BLACK);
    }

    // Roads
    for (int r = 0; r < NUM_ROADS; r++) {
        if (board.roads[r].owner == NONE) continue;
        Vector2 a = L.corners[ROAD_LINKS[r].corners[0]];
        Vector2 b = L.corners[ROAD_LINKS[r].corners[1]];
        a = Vector2Lerp(a, b, 0.15f);                    // shorten a bit so buildings stay visible
        b = Vector2Lerp(b, a, 0.15f / 0.85f);
        DrawLineEx(a, b, s * 0.14f, BLACK);
        DrawLineEx(a, b, s * 0.09f, playerColor(board.roads[r].owner));
    }

    // Settlements and cities
    for (int c = 0; c < NUM_CORNERS; c++) {
        const CornerState& cs = board.corners[c];
        if (cs.building == BuildingType::None) continue;
        float size = (cs.building == BuildingType::City ? 0.42f : 0.26f) * s;
        Rectangle rect = {L.corners[c].x - size / 2, L.corners[c].y - size / 2, size, size};
        DrawRectangleRec(rect, playerColor(cs.owner));
        DrawRectangleLinesEx(rect, 2, BLACK);
    }

    // Debug labels: same numbers as board_indexing.png (code index + 1)
    if (showIndices) {
        for (int t = 0; t < NUM_TILES; t++)
            drawCenteredText(TextFormat("T%d", t + 1),
                             {L.tileCenters[t].x, L.tileCenters[t].y + s * 0.55f}, 16, RAYWHITE);
        for (int r = 0; r < NUM_ROADS; r++) {
            Vector2 m = Vector2Lerp(L.corners[ROAD_LINKS[r].corners[0]], L.corners[ROAD_LINKS[r].corners[1]], 0.5f);
            DrawCircleV(m, 10, {25, 45, 60, 255});
            drawCenteredText(TextFormat("%d", r + 1), m, 10, WHITE);
        }
        for (int c = 0; c < NUM_CORNERS; c++) {
            DrawCircleV(L.corners[c], 11, WHITE);
            DrawCircleLinesV(L.corners[c], 11, {60, 160, 150, 255});
            drawCenteredText(TextFormat("%d", c + 1), L.corners[c], 10, {30, 90, 85, 255});
        }
        for (int i = 0; i < NUM_PORTS; i++){
            drawCenteredText(TextFormat("P%d", i + 1), {L.ports[i].x, L.ports[i].y + s * 0.4f}, 14, RAYWHITE);
        }
    }
}

int cornerAt(const BoardLayout& L, Vector2 p) {
    for (int c = 0; c < NUM_CORNERS; c++) {
        if (Vector2Distance(p, L.corners[c]) < L.hexSize * 0.25f) return c;
    }
    return NONE;
}

int roadAt(const BoardLayout& L, Vector2 p) {
    for (int r = 0; r < NUM_ROADS; r++) {
        Vector2 mid = Vector2Lerp(L.corners[ROAD_LINKS[r].corners[0]], L.corners[ROAD_LINKS[r].corners[1]], 0.5f);
        if (Vector2Distance(p, mid) < L.hexSize * 0.25f) return r;
    }
    return NONE;
}

int tileAt(const BoardLayout& L, Vector2 p) {
    for (int t = 0; t < NUM_TILES; t++)
        if (Vector2Distance(p, L.tileCenters[t]) < L.hexSize * 0.8f) return t;
    return NONE;
}

int portAt(const BoardLayout& L, Vector2 p) {
    for (int i = 0; i < NUM_PORTS; i++)
        if (Vector2Distance(p, L.ports[i]) < L.hexSize * 0.28f) return i;
    return NONE;
}

Color playerColor(int player) {
    static const Color colors[4] = { {220, 50, 47, 255}, {38, 110, 220, 255}, {245, 245, 245, 255}, {240, 140, 20, 255} };
    return (player >= 0 && player < 4) ? colors[player] : GRAY;
}

const char* resourceName(Resource r) {
    static const char* names[] = {"Wood", "Brick", "Wool", "Grain", "Ore", "Desert", "3:1"};
    return names[static_cast<int>(r)];
}

void drawResourceButtons(Vector2 origin) {
    Vector2 m = GetMousePosition();
    for (int i = 0; i < 5; i++) {
        Resource r = static_cast<Resource>(i);
        Rectangle rect = resourceButtonRect(origin, r);
        DrawRectangleRec(rect, resourceColor(r));
        DrawRectangleLinesEx(rect, 2, CheckCollisionPointRec(m, rect) ? YELLOW : BLACK);  // hover
        int w = MeasureText(resourceName(r), 12);
        DrawText(resourceName(r), (int)(rect.x + (BTN_SIZE - w) / 2), (int)(rect.y + BTN_SIZE + 4), 12, LIGHTGRAY);
    }
}

bool woodAt (Vector2 o, Vector2 p) { return CheckCollisionPointRec(p, resourceButtonRect(o, Resource::Wood));  }
bool brickAt(Vector2 o, Vector2 p) { return CheckCollisionPointRec(p, resourceButtonRect(o, Resource::Brick)); }
bool woolAt (Vector2 o, Vector2 p) { return CheckCollisionPointRec(p, resourceButtonRect(o, Resource::Wool));  }
bool grainAt(Vector2 o, Vector2 p) { return CheckCollisionPointRec(p, resourceButtonRect(o, Resource::Grain)); }
bool oreAt  (Vector2 o, Vector2 p) { return CheckCollisionPointRec(p, resourceButtonRect(o, Resource::Ore));   }