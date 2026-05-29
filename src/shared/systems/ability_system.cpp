#include "ability_system.hpp"

namespace System {

void AbilitySystem::Initialize(EventBus &eventBus, Map::MapData &map) {
    m_eventBus = &eventBus;
    m_map = &map;
};

void AbilitySystem::Update(float dt, std::array<state::PlayerState, MAX_PLAYERS> &players) {

    for (auto &player : players) {
        if (!player.active)
            continue;
        PlayerPickupCheck(player);
        TickPlayerEffects(dt, player);
        TickPlayerAbilities(dt, player);
    }

    SpawnPickup(dt);
}

void AbilitySystem::ClearAbilitiesAndEffects(state::PlayerState &player) {

    for (auto &effect : player.effects) {
        effect.type = state::EffectType::None;
        effect.category = state::EffectCategory::None;
        effect.active = false;
    }

    for (auto &ability : player.abilities) {
        ability.type = state::AbilityType::None;
        ability.active = false;
    }
}

void AbilitySystem::SpawnPickup(float dt) {
    m_abilityPickupTimer += dt;

    if (m_abilityPickupTimer < 5.0f)
        return;

    if (m_currentPickups.size() >= m_map->powerUpSpawns.size())
        return;

    std::vector<Vector2> availableSpawns;

    for (const auto &spawn : m_map->powerUpSpawns) {
        if (!IsSpawnOccupied(spawn, 10.0f)) {
            availableSpawns.push_back(spawn);
        }
    }

    if (availableSpawns.empty())
        return;

    const auto &powerUps = state::GetSpawnablePowerUps();
    state::SpawnablePickup powerUp = powerUps[GetRandomValue(0, powerUps.size() - 1)];
    Vector2 position = availableSpawns[GetRandomValue(0, availableSpawns.size() - 1)];
    m_currentPickups.emplace_back(m_abilityId, powerUp.pickupType, powerUp.typeId, Collision::Circle{position, 14.0f});

    if (m_eventBus) {
        m_eventBus->publish({m_abilityId, powerUp.pickupType, powerUp.typeId, position, 14.0f});
    }

    m_abilityId++;
    m_abilityPickupTimer = 0.0f;
}

bool AbilitySystem::IsSpawnOccupied(const Vector2 &position, float radius) const {
    for (const auto &pickup : m_currentPickups) {
        if (pickup.collider.center.x == position.x && pickup.collider.center.y == position.y) {
            return true;
        }
    }

    return false;
}

void AbilitySystem::PlayerPickupCheck(state::PlayerState &player) {
    for (auto it = m_currentPickups.begin(); it != m_currentPickups.end();) {
        if (Collision::Overlap(Collision::HurtboxToCircle(player.position, player.hurtbox), it->collider)) {
            AddPowerUp({it->pickupType, it->typeId}, player);
            it = RemovePickup(it);
        } else {
            ++it;
        }
    }
}

std::vector<state::AbilityPickup>::iterator
AbilitySystem::RemovePickup(std::vector<state::AbilityPickup>::iterator it) {
    m_abilityPickupTimer = 0.0f;
    if (m_eventBus) {
        event::PowerUpDespawnEvent despawn{.id = it->id};
        m_eventBus->publish(despawn);
    }

    return m_currentPickups.erase(it);
}

void AbilitySystem::TickPlayerEffects(float dt, state::PlayerState &player) {
    for (auto &e : player.effects) {
        if (!e.active)
            continue;

        e.durationRemaining -= dt;

        if (e.durationRemaining <= 0.0f) {
            e.active = false;
        }
    }
}

void AbilitySystem::TickPlayerAbilities(float dt, state::PlayerState &player) {
    for (auto &e : player.abilities) {
        if (!e.active)
            continue;

        e.durationRemaining -= dt;

        if (e.durationRemaining <= 0.0f) {
            e.active = false;
        }
    }
}

void AbilitySystem::AddPowerUp(state::SpawnablePickup powerUp, state::PlayerState &player) {
    switch (powerUp.pickupType) {

    case state::PickupType::Effect:
        ApplyEffect((state::EffectType)powerUp.typeId, player);
        break;

    case state::PickupType::Ability:
        ApplyAbility((state::AbilityType)powerUp.typeId, player);
        break;
    }
}

void AbilitySystem::ApplyAbility(const state::AbilityType &type, state::PlayerState &player) {
    const state::AbilityDefinition abilityDef = state::GetAbilityDefinition(type);

    for (auto &a : player.abilities) {
        if (!a.active)
            continue;

        if (a.type == type) {
            a.durationRemaining = abilityDef.baseDuration;
            return;
        }
    }

    state::ActiveAbility ability;
    ability.type = abilityDef.type;
    ability.durationRemaining = abilityDef.baseDuration;
    ability.maxDuration = abilityDef.baseDuration;
    ability.active = true;
    AddAbility(ability, player);
}

void AbilitySystem::AddAbility(const state::ActiveAbility &ability, state::PlayerState &player) {
    for (auto &a : player.abilities) {
        if (!a.active) {
            a = ability;
            return;
        }
    }
}

void AbilitySystem::ApplyDebuffs(state::PlayerState &target, state::PlayerState &attacker) {
    for (const auto &ability : attacker.abilities) {
        if (!ability.active)
            continue;

        const auto &def = GetAbilityDefinition(ability.type);

        if (def.appliesEffect) {
            ApplyEffect(def.appliedEffect, target);
        }
    }
}

void AbilitySystem::ApplyEffect(const state::EffectType &type, state::PlayerState &target) {
    const state::EffectDefinition effectDef = state::GetEffectDefinition(type);

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
    e.maxDuration = effectDef.baseDuration;
    e.magnitude = effectDef.baseMagnitude;
    e.active = true;

    AddEffect(e, target);
}

void AbilitySystem::AddEffect(const state::ActiveEffect &effect, state::PlayerState &player) {
    for (auto &e : player.effects) {
        if (!e.active) {
            e = effect;
            return;
        }
    }
}

} // namespace System
