#pragma once

#include "../../config.hpp"
#include "../../server/event_bus.hpp"
#include "../map/map_types.hpp"
#include "../state/active_effect.hpp"
#include "../state/player_state.hpp"
#include <array>
#include <vector>

namespace System {

class AbilitySystem {

  public:
    void Initialize(EventBus &eventBus, Map::MapData &map);
    void Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players);
    void ApplyEffect(state::PlayerState target, const state::EffectType type, const state::PlayerState attacker);

  private:
    void PlayerPickupCheck(state::PlayerState &player);
    std::vector<state::AbilityPickup>::iterator RemovePickup(std::vector<state::AbilityPickup>::iterator it);
    void TickPlayerEffects(float dt, state::PlayerState &player);

    void RemoveEffect(state::PlayerState &player);
    void AddEffect(state::EffectType type, state::PlayerState &player);
    void AddEffect(state::ActiveEffect effect, state::PlayerState &player);

  private:
    EventBus *m_eventBus;
    Map::MapData *m_map;
    std::vector<state::AbilityPickup> m_currentPickups;
    float m_abilityPickupTimer = 0.0f;

    uint32_t m_abilityId = 0;
};

} // namespace System
