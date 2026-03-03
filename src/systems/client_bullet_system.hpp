#pragma once

#include "bullet_system.hpp"
#include "state/bullet_state.hpp"
#include <array>

namespace System {

class ClientBulletSystem : public BulletSystem {

  public:
    int Spawn(uint32_t ownerId, Vector2 position, Vector2 direction, float speed = 400.0f,
              float lifetime = 3.0f) override;
    void Update(float dt) override;
    void Deactivate(int slot) override;
    std::array<state::ClientBulletState, MAX_BULLETS> &GetBullets();

  private:
    std::array<state::ClientBulletState, MAX_BULLETS> m_bullets{};
    std::array<uint16_t, MAX_BULLETS> m_generations{};
};
} // namespace System
