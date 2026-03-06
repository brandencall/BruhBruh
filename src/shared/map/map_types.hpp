#pragma once
#include "../../config.hpp"
#include "../components/collision.hpp"
#include <vector>

struct MapData {
    std::vector<Collision::AABB> walls;
    Vector2 spawnPoints[MAX_PLAYERS];
};
