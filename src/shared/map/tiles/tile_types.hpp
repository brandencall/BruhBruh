#pragma once
#include <cstdint>

namespace Map {

enum class TileType : uint8_t {
    Empty = 0,
    Wall = 1,
};

inline bool IsSolid(TileType t) { return t == TileType::Wall; }

} // namespace Map
