#pragma once

#include "../../shared/state/active_effect.hpp"
#include <unordered_map>
#include <vector>

namespace Render {

struct SpawnablePickupHash {
    std::size_t operator()(const state::SpawnablePickup &p) const {
        return (static_cast<size_t>(p.pickupType) << 8) | p.typeId;
    }
};

class AbilityPickupRenderer {
  public:
    void Load();
    void Unload();
    void Draw(std::vector<state::AbilityPickup> &pickups);

  private:
    std::unordered_map<state::SpawnablePickup, Texture2D, SpawnablePickupHash> m_textures;
};

} // namespace Render
