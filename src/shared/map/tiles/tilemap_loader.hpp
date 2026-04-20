#pragma once
#include "../map_def.hpp"
#include "tile_types.hpp"
#include <fstream>
#include <sstream>
#include <vector>

namespace Map {

struct TileMap {
    int width = 0;
    int height = 0;
    std::vector<TileType> tiles;         // row-major: tiles[y * width + x]
    const TilesetDef *tileset = nullptr; // non-owning, points into MapDef

    TileType At(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return TileType::Wall; // out-of-bounds = solid
        return tiles[y * width + x];
    }

    bool IsSolidAt(int x, int y) const {
        TileType t = At(x, y);
        uint8_t id = static_cast<uint8_t>(t);
        if (tileset && id < tileset->tiles.size())
            return tileset->tiles[id].solid;
        return IsSolid(t); // fallback to enum default
    }
};

inline TileMap LoadTileMap(const MapDef &def) {
    TileMap map;
    map.tileset = &def.tileset;

    std::ifstream file(def.mapPath);
    if (!file.is_open())
        return map;

    std::string line;
    int row = 0;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        int col = 0;
        // Tokenize by whitespace so tile IDs and S/I tokens are handled uniformly
        std::istringstream ss(line);
        std::string token;

        while (ss >> token) {
            TileType tile = TileType::Empty;

            if (token == "S")
                tile = TileType::Spawn;
            else if (token == "I")
                tile = TileType::InitialSpawn;
            else {
                int id = 0;
                try {
                    id = std::stoi(token);
                } catch (...) {
                }
                tile = static_cast<TileType>(id);
            }

            map.tiles.push_back(tile);
            col++;
        }

        if (map.width == 0)
            map.width = col;
        row++;
    }

    map.height = row;
    return map;
}

} // namespace Map
