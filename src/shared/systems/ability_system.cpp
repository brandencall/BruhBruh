#include "ability_system.hpp"

namespace System {

void AbilitySystem::Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) {
    for (auto &player : players) {
        for (auto it = m_currectPickups.begin(); it != m_currectPickups.end();) {
            if (Collision::Overlap(Collision::HurtboxToCircle(player.position, player.hurtbox), it->collider)) {
                AddEffect(it->type, player);
                it = m_currectPickups.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void AbilitySystem::ApplyEffect(state::PlayerState target, const EffectType type, const state::PlayerState attacker) {
    const EffectDefinition effectDef = GetEffectDefinition(type);

    for (auto &e : target.effects) {
        if (e.type == type) {
            e.durationRemaining = effectDef.baseDuration;
            e.magnitude = effectDef.baseMagnitude;
            return;
        }
    }

    ActiveEffect e;
    e.type = type;
    e.category = effectDef.category;
    e.durationRemaining = effectDef.baseDuration;
    e.magnitude = effectDef.baseMagnitude;
    e.sourcePlayerId = attacker.id;

    target.effects.push_back(e);
}

void AbilitySystem::AddEffect(EffectType type, state::PlayerState &player) {
    const EffectDefinition effectDef = GetEffectDefinition(type);
    player.effects.emplace_back(effectDef.type, effectDef.category, effectDef.baseDuration, effectDef.baseMagnitude,
                                player.id);
}

} // namespace System
