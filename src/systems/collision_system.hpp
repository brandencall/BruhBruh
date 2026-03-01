#pragma once

#include "../components/collider.hpp"
#include "entity.hpp"
#include <memory>
#include <vector>

namespace System {
class CollisionSystem {
  public:
    void Update(std::vector<std::unique_ptr<Entity>> &entities);
};

} // namespace System
