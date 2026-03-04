#include "client_bullet_system.hpp"
#include "raymath.h"

namespace System {

void ClientBulletSystem::OnSpawn(state::ClientBulletState &bullet, Vector2 spawnPos) {
    bullet.serverPosition = spawnPos;
}

void ClientBulletSystem::OnUpdate(state::ClientBulletState &bullet, float dt) {
    bullet.position = Vector2Lerp(bullet.position, bullet.serverPosition, BULLET_INTERP_SPEED * dt);
}

} // namespace System
