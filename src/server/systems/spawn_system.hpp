#pragma once

#include "raylib.h"
#include "state/player_state.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace System {

Vector2 InitSpawn(const std::vector<Vector2> &spawnPoints, uint32_t playerId);
Vector2 Spawn(const std::vector<Vector2> &spawnPoints, const std::array<state::PlayerState, MAX_PLAYERS> &players);

} // namespace System
