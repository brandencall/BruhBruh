#pragma once

#include "../components/collision.hpp"
#include <cmath>

namespace Map {

constexpr int GRID_CELL_SIZE = 64;

struct Vector2i {
    int x;
    int y;

    bool operator==(const Vector2i &other) const { return x == other.x && y == other.y; }
};

// May need to make this into a Vector2i intstead of a Vector2
inline Vector2i WorldToGrid(Vector2 worldPos) {
    return {(int)std::floor(worldPos.x / GRID_CELL_SIZE), (int)std::floor(worldPos.y / GRID_CELL_SIZE)};
}

inline Vector2 GridToWorld(Vector2i gridPos) {
    return {(float)(gridPos.x * GRID_CELL_SIZE), (float)(gridPos.y * GRID_CELL_SIZE)};
}

inline Collision::AABB GridCellToAABB(Vector2i gridPos) {
    Vector2 origin = GridToWorld(gridPos);
    return {origin, {origin.x + GRID_CELL_SIZE, origin.y + GRID_CELL_SIZE}};
}

} // namespace Map
