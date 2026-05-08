#pragma once
#include "../components/collision.hpp" // your existing collision header
#include "../state/player_state.hpp"
#include "character_roster.hpp"
#include "character_types.hpp"
#include "raymath.h"

namespace Character {

struct CollisionContext {
    const std::vector<Collision::AABB> *staticWalls;
    const std::vector<Collision::AABB> *dynamicWalls;
};

inline void SimulateMove(state::PlayerState &player, float dt, const CollisionContext *ctx = nullptr) {

    Vector2 dir = Vector2Normalize({player.currentInput.moveX, player.currentInput.moveY});
    const Character::CharacterDef &charDef = GetCharacterDef(player.characterId);
    player.velocity = Vector2Scale(dir, charDef.moveSpeed);
    player.position = Vector2Add(player.position, Vector2Scale(player.velocity, dt));

    if (ctx && ctx->staticWalls) {
        Collision::Circle circle = {player.position, player.hurtbox.radius};
        player.position = Collision::resolveCircleAABBList(circle, *ctx->staticWalls, *ctx->dynamicWalls);
    }
}

} // namespace Character
