#pragma once

#include "../../config.hpp"
#include "../components/collision.hpp"
#include "../state/active_effect.hpp"
#include "../state/player_state.hpp"
#include <array>

namespace System {

struct AbilityPickup {
    EffectType type;
    Collision::Circle collider;
};

class AbilitySystem {

  public:
    void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players);
    void ApplyEffect(state::PlayerState target, const EffectType type, const state::PlayerState attacker);

  private:
    void AddEffect(EffectType type, state::PlayerState &player);

    std::vector<AbilityPickup> m_currectPickups;
};

} // namespace System
