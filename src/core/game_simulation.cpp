#include "game_simulation.hpp"
#include "player_state.hpp"
#include "raymath.h"
#include <cstdint>
#include <iostream>

void GameSimulation::Update(float tickRate) {
    for (auto &player : m_players) {
        if (!player.active)
            continue;

        // convert input into velocity
        Vector2 direction = {0, 0};

        direction.y = player.currentInput.moveY;
        direction.x = player.currentInput.moveX;

        direction = Vector2Normalize(direction);

        player.velocity = direction * player.speed;

        // now advance simulation
        player.position += player.velocity * tickRate;
        std::cout << "Player " << player.id << " at position (" << player.position.x << ", " << player.position.y << ")"
                  << std::endl;
    }
}

void GameSimulation::ApplyInput(uint32_t playerId, const PlayerInput &input) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        m_players[playerId].currentInput = input;
    }
}

std::array<PlayerState, MAX_PLAYERS> GameSimulation::GetPlayers() { return m_players; }

std::vector<PlayerState> GameSimulation::GetActivePlayers() {
    // Filter out players who are not active
    std::vector<PlayerState> active_players;
    for (const auto &player : m_players) {
        if (player.active) {
            active_players.push_back(player);
        }
    }
    return active_players;
}

void GameSimulation::CreatePlayer(uint32_t playerId) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        PlayerState player = {
            .id = playerId,
            .active = true,
        };
        m_players[playerId] = player;
    }
}

void GameSimulation::RemovePlayer(uint32_t playerId) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        PlayerState &player = m_players[playerId];
        player.id = UINT32_MAX;
        player.active = false;
    }
}
