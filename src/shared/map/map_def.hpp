#pragma once
#include "tiles/tileset_def.hpp"
#include <string>

namespace Map {

struct MapDef {
    std::string mapPath; // "assets/maps/map01.txt"
    TilesetDef tileset;
};

} // namespace Map
