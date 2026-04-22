#pragma once

constexpr const float MATCH_TIME = 60 * 10;

constexpr const int MAX_PLAYERS = 4;
constexpr const int MAX_PLAYER_NAME_LEN = 32;

// TODO: This is probably not right
constexpr const int MAX_BULLETS = 256;
// TODO: Update this once the max walls per player is figured out
constexpr const int MAX_WALLS = 64;
constexpr const float RESPAWN_TIME = 3.0f;

// constexpr const char *MAP_PATH = "assets/maps/map01.txt";
#include "shared/map/map_registry.hpp"
inline const Map::MapDef &ACTIVE_MAP = Map::MAP01;
