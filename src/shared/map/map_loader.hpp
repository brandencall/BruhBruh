#pragma once
#include "grid.hpp"
#include "map_def.hpp"
#include "map_types.hpp"
#include "rect_merger.hpp"
#include "tiles/tilemap_loader.hpp"

namespace Map {

inline MapData LoadMap(const MapDef &def) {
    TileMap tileMap = LoadTileMap(def);

    MapData data;
    data.tileMap = std::move(tileMap);

    for (int y = 0; y < data.tileMap.height; y++) {
        for (int x = 0; x < data.tileMap.width; x++) {
            TileType &t = data.tileMap.tiles[y * data.tileMap.width + x];

            // World-space centre of this tile
            Vector2 worldPos = {(x + 0.5f) * GRID_CELL_SIZE, (y + 0.5f) * GRID_CELL_SIZE};

            if (t == TileType::InitialSpawn) {
                data.initialSpawns.push_back(worldPos);
                data.spawnPoints.push_back(worldPos);
                t = TileType::Empty; // strip so collision/render ignore it
            } else if (t == TileType::Spawn) {
                data.spawnPoints.push_back(worldPos);
                t = TileType::Empty;
            }
        }
    }

    data.walls = MergeToRects(data.tileMap);

    return data;
}

} // namespace Map
