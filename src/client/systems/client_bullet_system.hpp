#pragma once

#include "../../shared/systems/bullet_system.hpp"
#include "../state/client_bullet_state.hpp"

namespace System {

class ClientBulletSystem : public BulletSystem<state::ClientBulletState> {

  public:
    void AssignId(int slot, uint32_t id);
    void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) override;

  protected:
    void OnSpawn(state::ClientBulletState &bullet, Vector2 position) override;

  private:
    static constexpr float BULLET_INTERP_SPEED = 5.0f;
};
} // namespace System
