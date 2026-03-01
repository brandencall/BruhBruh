#include "../components/collision_components.hpp"
#include "entity.hpp"
#include <memory>
#include <vector>

namespace System {
class DamageSystem {
  public:
    void Update(std::vector<std::unique_ptr<Entity>> &entities);
};

} // namespace System
