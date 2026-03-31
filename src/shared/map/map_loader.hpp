#pragma once
#include "../../config.hpp"
#include "map_def.hpp"
#include "map_types.hpp"
#include "rect_merger.hpp"
#include "tiles/tilemap_loader.hpp"

namespace Map {

// Spawn points are still declared in a header section above the grid:
//   # SPAWN 0 x y
inline MapData LoadMap(const MapDef &def) {
    TileMap tileMap = LoadTileMap(def);

    MapData data;
    data.walls = MergeToRects(tileMap);
    data.tileMap = std::move(tileMap); // kept for WallManager::CanPlaceWall and renderer

    // Parse spawn points from a header block in the same file
    std::ifstream file(def.mapPath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] != '#')
            continue;
        std::istringstream ss(line.substr(1));
        std::string token;
        ss >> token;
        if (token == "SPAWN") {
            int idx;
            float x, y;
            ss >> idx >> x >> y;
            if (idx < MAX_PLAYERS)
                data.spawnPoints[idx] = {x, y};
        }
    }
    return data;
}

} // namespace Map
