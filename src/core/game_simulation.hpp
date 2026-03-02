#pragma once
#include "../config.hpp"
#include "../entities/player_state.hpp"
#include <array>
#include <stdint.h>

class GameSimulation {
  public:
    void Update(float tickRate);
    void ApplyInput(uint32_t playerId, const PlayerInput &input);
    std::array<PlayerState, MAX_PLAYERS> GetPlayers();
    void CreatePlayer(uint32_t playerId);
    void RemovePlayer(uint32_t playerId);

  private:
    std::array<PlayerState, MAX_PLAYERS> m_players;
};
