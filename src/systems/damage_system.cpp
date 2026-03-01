#include "damage_system.hpp"
#include "../entities/entity.hpp"
#include "collision_components.hpp"

namespace System {

void DamageSystem::Update(std::vector<std::unique_ptr<Entity>> &entities) {
    std::vector<Component::Hitbox *> hitboxes;
    std::vector<Component::Hurtbox *> hurtboxes;

    for (auto &e : entities) {
        if (e->IsDead())
            continue;

        if (auto *hit = e->GetHitbox())
            hitboxes.push_back(hit);

        if (auto *hurt = e->GetHurtbox())
            hurtboxes.push_back(hurt);
    }

    for (auto *hit : hitboxes) {
        for (auto *hurt : hurtboxes) {
            if (hurt->owner == hit->shooter)
                continue;

            if (hit->bounds.Overlaps(hurt->bounds)) {
                hurt->owner->ApplyDamage(hit->damage);
                hit->owner->SetDead();
            }
        }
    }
}

} // namespace System
