#include "collision_system.hpp"

namespace System {

void CollisionSystem::Update(std::vector<std::unique_ptr<Entity>> &entities) {
    for (size_t i = 0; i < entities.size(); i++) {
        for (size_t j = i + 1; j < entities.size(); j++) {

            if (entities[i]->GetCollider() == nullptr || entities[j]->GetCollider() == nullptr)
                continue;

            if (entities[i]->GetCollider()->bounds.Overlaps(entities[j]->GetCollider()->bounds)) {
                Entity *entityA = entities[i].get();
                Entity *entityB = entities[j].get();
                entityA->OnCollision(entityB);
                entityB->OnCollision(entityA);
            }
        }
    }
}
} // namespace System
