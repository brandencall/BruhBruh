#pragma once
#include "../../config.hpp"
#include "../components/collision.hpp"
#include "tiles/tilemap_loader.hpp"
#include <vector>

namespace Map {

struct MapData {
    std::vector<Collision::AABB> walls; // merged, used for collision
    TileMap tileMap;                    // raw grid, used for CanPlaceWall + rendering
    Vector2 spawnPoints[MAX_PLAYERS];
};

} // namespace Map
