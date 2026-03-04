#pragma once

#include "../../shared/systems/bullet_system.hpp"
#include "../state/client_bullet_state.hpp"

namespace System {

class ClientBulletSystem : public BulletSystem<state::ClientBulletState> {

  protected:
    void OnSpawn(state::ClientBulletState &bullet, Vector2 position) override;
    void OnUpdate(state::ClientBulletState &bullet, float dt) override;

  private:
    static constexpr float BULLET_INTERP_SPEED = 5.0f;
};
} // namespace System
