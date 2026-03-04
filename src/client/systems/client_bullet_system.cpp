#include "client_bullet_system.hpp"
#include "raylib.h"
#include "raymath.h"

namespace System {

void ClientBulletSystem::OnSpawn(state::ClientBulletState &bullet, Vector2 spawnPos) {
    bullet.serverPosition = spawnPos;
}

void ClientBulletSystem::OnUpdate(state::ClientBulletState &bullet, float dt) {
    Vector2 position =
        Vector2Lerp(Shapes::CircleToVector(bullet.hitbox.circle), bullet.serverPosition, BULLET_INTERP_SPEED * dt);
    bullet.hitbox.circle.x = position.x;
    bullet.hitbox.circle.y = position.y;
}

} // namespace System
