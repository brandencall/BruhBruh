
#pragma once
#include "../../config.hpp"
#include "../components/collision.hpp"
#include <array>
#include <cstdint>

namespace state {

enum class EffectType : uint8_t { None = 0, SpeedBoost, DamageBoost, Slow };

enum class EffectCategory : uint8_t { Buff, Debuff };

struct ActiveEffect {
    EffectType type;
    EffectCategory category;
    float durationRemaining;
    float magnitude;
    uint32_t sourcePlayerId;
    bool active = false;
};

struct EffectDefinition {
    EffectType type;
    EffectCategory category;
    float baseDuration;
    float baseMagnitude;
};

struct AbilityPickup {
    EffectType type;
    Collision::Circle collider;
};

inline const EffectDefinition &GetEffectDefinition(EffectType type) {
    static const EffectDefinition effect[] = {{},
                                              {EffectType::SpeedBoost, EffectCategory::Buff, 5.0f, 1.5f},
                                              {EffectType::DamageBoost, EffectCategory::Buff, 5.0f, 1.5f},
                                              {EffectType::Slow, EffectCategory::Debuff, 5.0f, .75f}};
    return effect[static_cast<uint8_t>(type)];
}

inline const std::array<EffectType, 2> &GetSpawnableEffects() {
    static const std::array<EffectType, 2> effects = {EffectType::SpeedBoost, EffectType::DamageBoost};

    return effects;
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

}; // namespace state
