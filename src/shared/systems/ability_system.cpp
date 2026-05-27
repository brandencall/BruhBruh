#include "ability_system.hpp"
#include <iostream>

namespace System {

void AbilitySystem::Initialize(EventBus &eventBus, Map::MapData &map) {
    m_eventBus = &eventBus;
    m_map = &map;
};

void AbilitySystem::Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) {

    static int frameCounter = 0;
    std::cout << "AbilitySystem Update call #" << frameCounter++ << std::endl;

    for (auto &player : players) {
        if (!player.active)
            continue;
        PlayerPickupCheck(player);
        TickPlayerEffects(dt, player);
    }

    m_abilityPickupTimer += dt;

    if (m_currentPickups.size() == 0 && m_abilityPickupTimer >= 5.0f) {
        // const auto &effects = state::GetSpawnableEffects();
        // state::EffectType effect = effects[GetRandomValue(0, effects.size() - 1)];
        state::EffectType effect = state::EffectType::SpeedBoost;
        Vector2 position = m_map->powerUpSpawns[GetRandomValue(0, m_map->powerUpSpawns.size() - 1)];
        m_currentPickups.emplace_back(effect, Collision::Circle{position, 10.0});
        if (m_eventBus)
            m_eventBus->publish({effect, position, 10.0f});

        m_abilityPickupTimer = 0.0f;
    }
}

void AbilitySystem::PlayerPickupCheck(state::PlayerState &player) {
    std::cout << "Pickup check for player " << player.id << "\n";
    for (auto it = m_currentPickups.begin(); it != m_currentPickups.end();) {
        if (Collision::Overlap(Collision::HurtboxToCircle(player.position, player.hurtbox), it->collider)) {
            AddEffect(it->type, player);
            std::cout << "Player: " << player.id << " picked up effect" << std::endl;
            it = m_currentPickups.erase(it);
            m_abilityPickupTimer = 0.0f;
        } else {
            ++it;
        }
    }
}

void AbilitySystem::TickPlayerEffects(float dt, state::PlayerState &player) {
    int count = 0;
    for (auto &e : player.effects) {
        if (!e.active)
            continue;

        if (e.active && e.type == state::EffectType::SpeedBoost)
            count++;

        e.durationRemaining -= dt;

        if (e.durationRemaining <= 0.0f) {
            e.active = false;
        }
    }
    std::cout << "SpeedBoost count: " << count << "\n";
}

void AbilitySystem::AddEffect(state::EffectType type, state::PlayerState &player) {
    const state::EffectDefinition effectDef = GetEffectDefinition(type);
    for (auto &e : player.effects) {
        if (!e.active) {
            e = {.type = effectDef.type,
                 .category = effectDef.category,
                 .durationRemaining = effectDef.baseDuration,
                 .magnitude = effectDef.baseMagnitude,
                 .sourcePlayerId = player.id,
                 .active = true};
            return;
        }
    }
}

void AbilitySystem::ApplyEffect(state::PlayerState target, const state::EffectType type,
                                const state::PlayerState attacker) {
    const state::EffectDefinition effectDef = GetEffectDefinition(type);

    for (auto &e : target.effects) {
        if (!e.active)
            continue;

        if (e.type == type) {
            e.durationRemaining = effectDef.baseDuration;
            e.magnitude = effectDef.baseMagnitude;
            return;
        }
    }

    state::ActiveEffect e;
    e.type = type;
    e.category = effectDef.category;
    e.durationRemaining = effectDef.baseDuration;
    e.magnitude = effectDef.baseMagnitude;
    e.sourcePlayerId = attacker.id;
    e.active = true;

    AddEffect(e, target);
}

void AbilitySystem::AddEffect(state::ActiveEffect effect, state::PlayerState &player) {
    for (auto &e : player.effects) {
        if (!e.active) {
            e = effect;
        }
    }
}

} // namespace System
