#pragma once
#include "../components/collision.hpp"
#include "../state/player_state.hpp"
#include "character_roster.hpp"
#include "character_types.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cstdint>
#include <vector>

namespace Character {

static inline void SimulateDrunkenness(Vector2 &dir, uint32_t playerId, const state::ActiveEffect *effect) {
    float t = effect->elapsedTime;

    // Stronger wobble
    float wobble = sinf(t * 9.0f + playerId) + sinf(t * 17.0f + playerId * 2.0f) * 0.75f +
                   sinf(t * 31.0f + playerId * 3.0f) * 0.5f;

    float angleOffset = wobble * DEG2RAD * 25.0f;

    float c = cosf(angleOffset);
    float s = sinf(angleOffset);

    dir = {dir.x * c - dir.y * s, dir.x * s + dir.y * c};

    // Mild continuous drift
    Vector2 right = {-dir.y, dir.x};

    float drift = sinf(t * 15.0f + playerId) * 0.25f;

    dir = Vector2Normalize({dir.x + right.x * drift, dir.y + right.y * drift});

    // Stumble impulse
    int stumbleStep = static_cast<int>(t / 0.15f);

    uint32_t hash = playerId ^ stumbleStep;
    hash ^= hash >> 16;
    hash *= 0x7feb352d;
    hash ^= hash >> 15;
    hash *= 0x846ca68b;
    hash ^= hash >> 16;

    float stumbleAmount = ((((hash & 0xFFFF) / 65535.0f) * 2.0f) - 1.0f) * 1.25f;

    dir = Vector2Normalize({dir.x + right.x * stumbleAmount, dir.y + right.y * stumbleAmount});
}

inline void SimulateMove(state::PlayerState &player, float dt, const std::vector<Collision::AABB> &staticWalls = {},
                         const std::vector<Collision::AABB> &dynamicWalls = {}) {

    Vector2 dir = Vector2Normalize({player.currentInput.moveX, player.currentInput.moveY});

    const state::ActiveEffect *drunkenness = state::GetActiveEffect(state::EffectType::Drunkenness, player.effects);
    if (drunkenness) {
        SimulateDrunkenness(dir, player.id, drunkenness);
    }

    const Character::CharacterDef &charDef = GetCharacterDef(player.characterId);
    float speed = charDef.moveSpeed * state::GetMovementMultiplier(player.effects);

    Vector2 targetVelocity = Vector2Scale(dir, speed);

    // Smoothly accelerate toward target velocity
    player.velocity = targetVelocity;

    player.position = Vector2Add(player.position, Vector2Scale(player.velocity, dt));

    Collision::Circle circle = {player.position, player.hurtbox.radius};
    player.position = Collision::resolveCircleAABBList(circle, staticWalls, dynamicWalls);
}

} // namespace Character
