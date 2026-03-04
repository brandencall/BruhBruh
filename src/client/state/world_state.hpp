#pragma once

#include "../../config.hpp"
#include "../shared/state/bullet_state.hpp"
#include "../shared/state/player_state.hpp"
#include "entities/render_player.hpp"
#include <array>
#include <cstdint>
#include <unordered_map>

// This might just need to be a struct instead of a class
class ClientWorldState {
  public:
    uint32_t m_currentPlayerId;
    std::unordered_map<uint32_t, RenderPlayer> m_renderPlayers;
    std::unordered_map<uint32_t, state::PlayerState> m_serverState;
    std::unordered_map<uint32_t, state::BulletState> m_serverBullets;
    std::array<state::PlayerState, MAX_PLAYERS> m_players;
};
