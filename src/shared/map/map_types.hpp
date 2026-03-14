#pragma once

#include "../../config.hpp"
#include "../components/collision.hpp"
#include <vector>

namespace Map {

struct MapData {
    std::vector<Collision::AABB> walls;
    Vector2 spawnPoints[MAX_PLAYERS];
};

} // namespace Map
