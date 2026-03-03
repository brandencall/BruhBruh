#pragma once
#include "./state/player_state.hpp"

class RenderPlayer {
  public:
    void SyncFromState(const state::PlayerState &state);
    void Draw(const state::PlayerState &state);
};
