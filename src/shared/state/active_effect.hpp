#pragma once

#include "../../config.hpp"
#include "../components/collision.hpp"
#include "raylib.h"
#include <array>
#include <cstdint>

namespace state {

enum class EffectType : uint8_t { None = 0, SpeedBoost, DamageBoost, Slow, Drunkenness, RapidFire };

enum class EffectCategory : uint8_t { None = 0, Buff, Debuff };

enum class AbilityType : uint8_t { None = 0, SlowShot, DrunkShot };

struct ActiveEffect {
    EffectType type;
    EffectCategory category;
    float durationRemaining;
    float maxDuration;
    float elapsedTime;
    float magnitude;
    bool active = false;
};

struct EffectDefinition {
    EffectType type = EffectType::None;
    EffectCategory category = EffectCategory::None;
    float baseDuration;
    float baseMagnitude;
    Color color = BLANK;
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
    static const EffectDefinition effect[] = {
        {},
        {
            EffectType::SpeedBoost, EffectCategory::Buff, 5.0f, 1.5f, {255, 200, 50, 225} // yellow
        },
        {
            EffectType::DamageBoost, EffectCategory::Buff, 5.0f, 1.5f, {255, 80, 80, 225} // red
        },
        {
            EffectType::Slow, EffectCategory::Debuff, 5.0f, 0.75f, {80, 150, 255, 225} // blue
        },
        {
            // TODO: implement some sort of magnitude to the Drunkenness
            EffectType::Drunkenness,
            EffectCategory::Debuff,
            5.0f,
            0.0f,
            {0, 255, 0, 225} // blue
        },
        {
            EffectType::RapidFire, EffectCategory::Buff, 5.0f, 0.1f, {236, 88, 0, 225} // Persimmon (orange)
        },
    };
    return effect[static_cast<uint8_t>(type)];
}

inline const AbilityDefinition &GetAbilityDefinition(AbilityType type) {
    static const AbilityDefinition abilities[] = {{},
                                                  {AbilityType::SlowShot, 5.0f, true, EffectType::Slow},
                                                  {AbilityType::DrunkShot, 5.0f, true, EffectType::Drunkenness}};
    return abilities[static_cast<uint8_t>(type)];
}

inline const std::array<SpawnablePickup, 5> &GetSpawnablePowerUps() {
    static const std::array<SpawnablePickup, 5> powerups = {{
        {PickupType::Effect, (uint8_t)EffectType::SpeedBoost},
        {PickupType::Effect, (uint8_t)EffectType::DamageBoost},
        {PickupType::Effect, (uint8_t)EffectType::RapidFire},
        {PickupType::Ability, (uint8_t)AbilityType::SlowShot},
        {PickupType::Ability, (uint8_t)AbilityType::DrunkShot},
    }};
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

inline const ActiveEffect *GetActiveEffect(EffectType type,
                                           const std::array<ActiveEffect, MAX_ACTIVE_EFFECTS> &effects) {
    for (const auto &effect : effects) {
        if (!effect.active)
            continue;
        if (effect.type == type)
            return &effect;
    }
    return nullptr;
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
