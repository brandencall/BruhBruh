#include "collision_system.hpp"

namespace System {
void CollisionSystem::AddCollider(Component::Collider *collider) { m_colliders.push_back(collider); }

void CollisionSystem::Update() {
    for (size_t i = 0; i < m_colliders.size(); i++) {
        for (size_t j = i + 1; j < m_colliders.size(); j++) {
            if (m_colliders[i]->bounds.Overlaps(m_colliders[j]->bounds)) {
                Entity *entityA = m_colliders[i]->owner;
                Entity *entityB = m_colliders[j]->owner;
                entityA->OnCollision(entityB);
                entityB->OnCollision(entityA);
            }
        }
    }
}
} // namespace System
