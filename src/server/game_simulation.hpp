#pragma once
#include "../config.hpp"
#include "../shared/state/player_state.hpp"
#include "../shared/systems/bullet_system.hpp"
#include <array>
#include <stdint.h>
#include <vector>

class GameSimulation {
  public:
    void Update(float tickRate);
    void ApplyInput(uint32_t playerId, const state::PlayerInput &input);
    const std::array<state::PlayerState, MAX_PLAYERS> &GetPlayers();
    const std::array<state::BulletState, MAX_BULLETS> &GetBullets();
    void CreatePlayer(uint32_t playerId);
    void RemovePlayer(uint32_t playerId);
    std::vector<state::PlayerState> GetActivePlayers();

  private:
    std::array<state::PlayerState, MAX_PLAYERS> m_players;
    System::BulletSystem<state::BulletState> m_bulletSystem;
};
