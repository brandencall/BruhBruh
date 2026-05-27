#pragma once
#include "../components/collision.hpp" // your existing collision header
#include "../state/player_state.hpp"
#include "character_roster.hpp"
#include "character_types.hpp"
#include "raymath.h"
#include <vector>

namespace Character {

inline void SimulateMove(state::PlayerState &player, float dt, const std::vector<Collision::AABB> &staticWalls = {},
                         const std::vector<Collision::AABB> &dynamicWalls = {}) {

    Vector2 dir = Vector2Normalize({player.currentInput.moveX, player.currentInput.moveY});
    const Character::CharacterDef &charDef = GetCharacterDef(player.characterId);
    float speed = charDef.moveSpeed * state::GetMovementMultiplier(player.effects);
    player.velocity = Vector2Scale(dir, speed);
    player.position = Vector2Add(player.position, Vector2Scale(player.velocity, dt));

    Collision::Circle circle = {player.position, player.hurtbox.radius};
    player.position = Collision::resolveCircleAABBList(circle, staticWalls, dynamicWalls);
}

} // namespace Character
