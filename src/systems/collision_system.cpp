#include "collision_system.hpp"

void CollisionSystem::AddCollider(Collider *collider) { m_colliders.push_back(collider); }

void CollisionSystem::Update() {
    for (size_t i = 0; i < m_colliders.size(); i++) {
        for (size_t j = i + 1; j < m_colliders.size(); j++) {
            CheckPair(m_colliders[i], m_colliders[j]);
        }
    }
}

bool CollisionSystem::CheckAABB(const AABB &a, const AABB &b) {
    return (a.x < b.x + b.width && a.x + a.width > b.x && a.y < b.y + b.height && a.y + a.height > b.y);
}

void CollisionSystem::CheckPair(const Collider *a, const Collider *b) {
    if (CheckAABB(a->bounds, b->bounds)) {
        Entity *entityA = a->owner;
        Entity *entityB = b->owner;
        entityA->OnCollision(entityB);
        entityB->OnCollision(entityA);
    }
}
