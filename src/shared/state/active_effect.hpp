#pragma once

#include "../../config.hpp"
#include "../components/collision.hpp"
#include <array>
#include <cstdint>

namespace state {

enum class EffectType : uint8_t { None = 0, SpeedBoost, DamageBoost, Slow };

enum class EffectCategory : uint8_t { None = 0, Buff, Debuff };

enum class AbilityType : uint8_t { None = 0, SlowShot };

struct ActiveEffect {
    EffectType type;
    EffectCategory category;
    float durationRemaining;
    float maxDuration;
    float magnitude;
    bool active = false;
};

struct EffectDefinition {
    EffectType type;
    EffectCategory category;
    float baseDuration;
    float baseMagnitude;
};

struct ActiveAbility {
    AbilityType type;
    float durationRemaining;
    float maxDuration;
    bool active = false;
};

struct AbilityDefinition {
    AbilityType type;
    float baseDuration;
    bool appliesEffect = false;
    EffectType appliedEffect = EffectType::None;
};

enum class PickupType : uint8_t { Effect, Ability };

struct SpawnablePickup {
    PickupType pickupType;
    uint8_t typeId;

    bool operator==(const SpawnablePickup &other) const {
        return pickupType == other.pickupType && typeId == other.typeId;
    }
};

struct AbilityPickup {
    uint32_t id;
    PickupType pickupType;
    uint8_t typeId;
    Collision::Circle collider;
};

inline const EffectDefinition &GetEffectDefinition(EffectType type) {
    static const EffectDefinition effect[] = {{},
                                              {EffectType::SpeedBoost, EffectCategory::Buff, 5.0f, 1.5f},
                                              {EffectType::DamageBoost, EffectCategory::Buff, 5.0f, 1.5f},
                                              {EffectType::Slow, EffectCategory::Debuff, 5.0f, .75f}};
    return effect[static_cast<uint8_t>(type)];
}

inline const AbilityDefinition &GetAbilityDefinition(AbilityType type) {
    static const AbilityDefinition abilities[] = {{}, {AbilityType::SlowShot, 5.0f, true, EffectType::Slow}};
    return abilities[static_cast<uint8_t>(type)];
}

inline const std::array<SpawnablePickup, 3> &GetSpawnablePowerUps() {
    static const std::array<SpawnablePickup, 3> powerups = {{{PickupType::Effect, (uint8_t)EffectType::SpeedBoost},
                                                             {PickupType::Effect, (uint8_t)EffectType::DamageBoost},
                                                             {PickupType::Ability, (uint8_t)AbilityType::SlowShot}}};
    return powerups;
}

inline const bool HasEffect(EffectType type, const std::array<ActiveEffect, MAX_ACTIVE_EFFECTS> &effects) {
    for (const auto &effect : effects) {
        if (!effect.active)
            continue;

        if (effect.type == type)
            return true;
    }
    return false;
}

inline float GetMovementMultiplier(const std::array<ActiveEffect, MAX_ACTIVE_EFFECTS> &effects) {
    float mult = 1.0f;

    for (const auto &effect : effects) {
        if (!effect.active)
            continue;

        switch (effect.type) {
        case EffectType::SpeedBoost:
            mult *= effect.magnitude;
            break;
        case EffectType::Slow:
            mult *= effect.magnitude;
            break;
        default:
            break;
        }
    }

    return mult;
}

inline float GetOverallDamage(float baseDamage, const std::array<ActiveEffect, MAX_ACTIVE_EFFECTS> &effects) {
    float finalDamage = baseDamage;
    const state::ActiveEffect *damageBoost = nullptr;

    for (const auto &effect : effects) {
        if (!effect.active)
            continue;

        // TODO: fill in switch when other damage is implemented
        switch (effect.type) {
        case EffectType::DamageBoost:
            damageBoost = &effect;
            break;
        default:
            break;
        }
    }
    // Apply damage boost after all damage effects have been aplied to base damage
    if (damageBoost)
        finalDamage *= damageBoost->magnitude;

    return finalDamage;
}
}; // namespace state
