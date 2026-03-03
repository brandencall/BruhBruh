#include "game_simulation.hpp"
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

        player.position += player.velocity * tickRate;
    }
}

void GameSimulation::ApplyInput(uint32_t playerId, const state::PlayerInput &input) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        m_players[playerId].currentInput = input;
    }
}

std::array<state::PlayerState, MAX_PLAYERS> GameSimulation::GetPlayers() { return m_players; }

std::vector<state::PlayerState> GameSimulation::GetActivePlayers() {
    std::vector<state::PlayerState> active_players;
    for (const auto &player : m_players) {
        if (player.active) {
            active_players.push_back(player);
        }
    }
    return active_players;
}

void GameSimulation::CreatePlayer(uint32_t playerId) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        state::PlayerState player = {
            .id = playerId,
            .active = true,
        };
        m_players[playerId] = player;
    }
}

void GameSimulation::RemovePlayer(uint32_t playerId) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        state::PlayerState &player = m_players[playerId];
        player.id = UINT32_MAX;
        player.active = false;
    }
}
