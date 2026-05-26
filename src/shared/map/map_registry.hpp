#pragma once
#include "map_def.hpp"

namespace Map {

inline const MapDef MAP01 = {.mapPath = "assets/maps/map01.txt",
                             .tileset = {.texturePath = "assets/tilesets/map01.png",
                                         .tileSize = 16,
                                         .columns = 8,
                                         .tiles = {
                                             {.solid = false}, // 0 — floor
                                             {.solid = true},  // 1 — wall
                                         }}};

inline const MapDef MAP02 = {.mapPath = "assets/maps/map02.txt",
                             .tileset = {.texturePath = "assets/tilesets/map01.png",
                                         .tileSize = 16,
                                         .columns = 8,
                                         .tiles = {
                                             {.solid = false}, // 0 — floor
                                             {.solid = true},  // 1 — wall
                                         }}};

} // namespace Map
