#pragma once
#include "player_state.hpp"
#include <cstdint>

class RenderPlayer {
  public:
    RenderPlayer(uint32_t id);
    void Sync(const PlayerState &state);
    void Draw();
    // Vector2 GetPosition() const override;
    Vector2 GetPosition() const;

  private:
    uint32_t m_id;
    bool m_isDead = false;
    float m_width = 32.0f;
    float m_height = 32.0f;
    Vector2 m_position;
    bool m_active;
};
