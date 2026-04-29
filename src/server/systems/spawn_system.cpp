#include "spawn_system.hpp"
#include "raymath.h"
#include <cassert>
#include <cstdint>

namespace System {

Vector2 InitSpawn(const std::vector<Vector2> &spawnPoints, uint32_t playerId) {
    assert(spawnPoints.size() - 1 >= playerId);
    return spawnPoints[playerId];
}

Vector2 Spawn(const std::vector<Vector2> &spawnPoints, const std::array<state::PlayerState, MAX_PLAYERS> &players) {
    Vector2 bestSpawn = spawnPoints[0];
    float bestScore = -1.0f;

    for (const auto &spawn : spawnPoints) {
        float spawnScore = 0.0f;

        for (const auto &player : players) {
            if (!player.active || player.health <= 0)
                continue;
            spawnScore += Vector2Distance(spawn, player.position);
        }

        if (spawnScore > bestScore) {
            bestScore = spawnScore;
            bestSpawn = spawn;
        }
    }

    return bestSpawn;
}

} // namespace System
