#pragma once

#include "../components/collider.hpp"
#include <vector>

namespace System {
class CollisionSystem {
  public:
    void AddCollider(Component::Collider *collider);
    void Update();

  public:
    std::vector<Component::Collider *> m_colliders;
};

} // namespace System
