
#pragma once
#include <cstdint>

namespace System {
enum class EffectType : uint8_t { SpeedBoost = 0, DamageBoost = 1 };

enum class EffectCategory : uint8_t { Buff, Debuff };

struct ActiveEffect {
    EffectType type;
    EffectCategory category;
    float durationRemaining;
    float magnitude;
    uint32_t sourcePlayerId;
};

struct EffectDefinition {
    EffectType type;
    EffectCategory category;
    float baseDuration;
    float baseMagnitude;
};

inline const EffectDefinition &GetEffectDefinition(EffectType type) {
    static const EffectDefinition effect[] = {{EffectType::SpeedBoost, EffectCategory::Buff, 5.0f, 1.5f},
                                              {EffectType::DamageBoost, EffectCategory::Buff, 5.0f, 1.5f}};
    return effect[static_cast<uint8_t>(type)];
}

}; // namespace System
