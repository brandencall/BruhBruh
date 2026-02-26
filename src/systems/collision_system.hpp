#pragma once

#include "../components/collider.hpp"
#include <vector>

class CollisionSystem {
  public:
    void AddCollider(Collider *collider);
    void Update();

  public:
    std::vector<Collider *> m_colliders;

  private:
    bool CheckAABB(const AABB &a, const AABB &b);
    void CheckPair(const Collider *a, const Collider *b);
};
