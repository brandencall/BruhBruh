#include "../components/collision_components.hpp"
#include <vector>

namespace System {
class DamageSystem {
  public:
    void AddHitbox(Component::Hitbox *hitbox);
    void AddHurtbox(Component::Hurtbox *hurtbox);
    void RemoveHitbox(Component::Hitbox *hitbox);
    void RemoveHurtbox(Component::Hurtbox *hurtbox);
    void Update();

  public:
    std::vector<Component::Hitbox *> m_hitboxes;
    std::vector<Component::Hurtbox *> m_hurtboxes;
};

} // namespace System
