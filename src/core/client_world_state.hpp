#pragma once

#include "../entities/render_player.hpp"
#include "../entities/state/player_state.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>

// This might just need to be a struct instead of a class
class ClientWorldState {
  public:
    uint32_t m_currentPlayerId;
    std::unordered_map<uint32_t, RenderPlayer> m_renderPlayers;
    std::unordered_map<uint32_t, state::PlayerState> m_serverState;
    std::vector<state::PlayerState> m_players;
};
