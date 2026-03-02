#pragma once
#include "player_state.hpp"

class RenderPlayer {
  public:
    void SyncFromState(const PlayerState &state);
    void Draw();
};
