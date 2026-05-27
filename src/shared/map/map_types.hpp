#pragma once
#include "../components/collision.hpp"
#include "tiles/tilemap_loader.hpp"
#include <vector>

namespace Map {

struct MapData {
    std::vector<Collision::AABB> walls; // merged, used for collision
    TileMap tileMap;                    // raw grid, used for CanPlaceWall + rendering
    std::vector<Vector2> spawnPoints;
    std::vector<Vector2> initialSpawns;
    std::vector<Vector2> powerUpSpawns;
};

} // namespace Map
