#include "game_simulation.hpp"
#include "../config.hpp"
#include "../shared/map/map_loader.hpp"
#include "raymath.h"
#include <cstdint>

void GameSimulation::Initialize() {
    m_players = {};
    m_map = LoadMap(MAP_PATH);
    m_bulletSystem.SetMap(m_map);
}

void GameSimulation::Update(float tickRate) {
    for (auto &player : m_players) {
        if (!player.active)
            continue;

        Vector2 dir = Vector2Normalize({player.currentInput.moveX, player.currentInput.moveY});
        player.velocity = Vector2Scale(dir, player.speed);
        player.position = Vector2Add(player.position, Vector2Scale(player.velocity, tickRate));
        Collision::Circle circle = {player.position, player.hurtbox.radius};
        player.position = Collision::resolveCircleAABBList(circle, m_map.walls);
    }

    m_bulletSystem.Update(tickRate, m_players);
}

const std::array<state::BulletState, MAX_BULLETS> &GameSimulation::GetBullets() { return m_bulletSystem.GetBullets(); }

System::BulletSystem<state::BulletState> &GameSimulation::GetBulletSystem() { return m_bulletSystem; }

void GameSimulation::ApplyInput(uint32_t playerId, const state::PlayerInput &input) {
    if (playerId < 0 || playerId > MAX_PLAYERS)
        return;

    state::PlayerState &player = m_players[playerId];
    bool shootNow = input.buttons & (1 << 0);
    bool shootPrev = player.lastButtons & (1 << 0);
    if (shootNow && !shootPrev) {
        Vector2 aimDir = {input.aimX, input.aimY};
        m_bulletSystem.Spawn(player.id, player.position, aimDir);
    }

    player.currentInput = input;
    player.lastButtons = input.buttons;
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
        Vector2 spawn = m_map.spawnPoints[playerId];
        state::PlayerState player = {
            .id = playerId, .position = spawn, .hurtbox = hurtbox, .active = true, .lastButtons = 0};
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
