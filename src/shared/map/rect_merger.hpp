#pragma once
#include "../components/collision.hpp"
#include "grid.hpp"
#include "tiles/tilemap_loader.hpp"

namespace Map {

inline std::vector<Collision::AABB> MergeToRects(const TileMap &map) {
    std::vector<bool> consumed(map.width * map.height, false);

    auto idx = [&](int x, int y) { return y * map.width + x; };
    auto isSolid = [&](int x, int y) { return !consumed[idx(x, y)] && map.IsSolidAt(x, y); };

    std::vector<Collision::AABB> result;

    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            if (!isSolid(x, y))
                continue;

            int w = 1;
            while (x + w < map.width && isSolid(x + w, y))
                w++;

            int h = 1;
            while (y + h < map.height) {
                bool fits = true;
                for (int col = x; col < x + w; col++)
                    if (!isSolid(col, y + h)) {
                        fits = false;
                        break;
                    }
                if (!fits)
                    break;
                h++;
            }

            for (int row = y; row < y + h; row++)
                for (int col = x; col < x + w; col++)
                    consumed[idx(col, row)] = true;

            result.push_back({{(float)(x * GRID_CELL_SIZE), (float)(y * GRID_CELL_SIZE)},
                              {(float)((x + w) * GRID_CELL_SIZE), (float)((y + h) * GRID_CELL_SIZE)}});
        }
    }
    return result;
}

} // namespace Map
