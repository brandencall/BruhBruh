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

class AbilityRenderer {
  public:
    void Load();
    void Unload();
    void DrawPickUps(std::vector<state::AbilityPickup> &pickups);
    const std::unordered_map<state::SpawnablePickup, Texture2D, SpawnablePickupHash> &GetTextures() const;

  private:
    std::unordered_map<state::SpawnablePickup, Texture2D, SpawnablePickupHash> m_textures;
};

} // namespace Render
