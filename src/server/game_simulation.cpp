#include "game_simulation.hpp"
#include "raymath.h"
#include <cstdint>

void GameSimulation::Update(float tickRate) {
    for (auto &player : m_players) {
        if (!player.active)
            continue;

        Vector2 dir = Vector2Normalize({player.currentInput.moveX, player.currentInput.moveY});
        player.velocity = Vector2Scale(dir, player.speed);
        player.position = Vector2Add(player.position, Vector2Scale(player.velocity, tickRate));

        bool shootNow = player.currentInput.buttons & (1 << 0);
        bool shootPrev = player.lastButtons & (1 << 0);
        if (shootNow && !shootPrev) {
            Vector2 aimDir = {player.currentInput.aimX, player.currentInput.aimY};
            m_bulletSystem.Spawn(player.id, player.position, aimDir);
        }
        player.lastButtons = player.currentInput.buttons;
    }

    m_bulletSystem.Update(tickRate, m_players);
}

const std::array<state::BulletState, MAX_BULLETS> &GameSimulation::GetBullets() { return m_bulletSystem.GetBullets(); }

void GameSimulation::ApplyInput(uint32_t playerId, const state::PlayerInput &input) {
    if (playerId >= 0 && playerId < MAX_PLAYERS) {
        m_players[playerId].currentInput = input;
    }
}

const std::array<state::PlayerState, MAX_PLAYERS> &GameSimulation::GetPlayers() { return m_players; }

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
        component::Hurtbox hurtbox = {.radius = 16.0f};
        state::PlayerState player = {
            .id = playerId,
            .hurtbox = hurtbox,
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
