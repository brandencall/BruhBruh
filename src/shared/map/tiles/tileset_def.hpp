#pragma once
#include <string>
#include <vector>

namespace Map {

struct TileDef {
    bool solid = false;
};

struct TilesetDef {
    std::string texturePath;
    // pixel size of one tile in the sheet
    int tileSize;
    // how many tiles wide the sheet is
    int columns;
    // indexed by tile ID
    std::vector<TileDef> tiles;
};

} // namespace Map
