#include "damage_system.hpp"
#include "collision_components.hpp"
#include <algorithm>

namespace System {

void DamageSystem::AddHurtbox(Component::Hurtbox *hurtbox) { m_hurtboxes.push_back(hurtbox); }

void DamageSystem::AddHitbox(Component::Hitbox *hitbox) { m_hitboxes.push_back(hitbox); }

void DamageSystem::RemoveHitbox(Component::Hitbox *h) {
    m_hitboxes.erase(std::remove(m_hitboxes.begin(), m_hitboxes.end(), h), m_hitboxes.end());
}

void DamageSystem::RemoveHurtbox(Component::Hurtbox *h) {
    m_hurtboxes.erase(std::remove(m_hurtboxes.begin(), m_hurtboxes.end(), h), m_hurtboxes.end());
}

void DamageSystem::Update() {
    for (Component::Hitbox *&hit : m_hitboxes) {
        for (Component::Hurtbox *&hurt : m_hurtboxes) {
            if (hurt->owner == hit->shooter)
                continue;

            if (hit->bounds.Overlaps(hurt->bounds))
                hurt->owner->ApplyDamage(hit->damage);
        }
    }
}

} // namespace System
