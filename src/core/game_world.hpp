#pragma once

#include "../entities/state/player_state.hpp"
#include <vector>
class ClientWorldState {

  private:
    std::vector<state::PlayerState> m_players;
};
