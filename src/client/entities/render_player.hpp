#pragma once

#include "../../shared/state/player_state.hpp"
#include "raylib.h"
#include <cstdint>

class RenderPlayer {
  public:
    RenderPlayer(uint32_t id);
    void Sync(const state::PlayerState &state, float dt);
    void Draw();
    Vector2 GetPosition() const;
    void SnapToPosition(const Vector2 &position);

  private:
    uint32_t m_id;
    float m_radius;
    bool m_isDead = false;
    Vector2 m_position;
    Vector2 m_targetPosition;
    bool m_active;
};
