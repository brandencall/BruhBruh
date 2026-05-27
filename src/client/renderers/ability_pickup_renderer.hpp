#pragma once

#include "../../shared/state/active_effect.hpp"
#include <unordered_map>
#include <vector>

namespace Render {

class AbilityPickupRenderer {
  public:
    void Load();
    void Unload();
    void Draw(std::vector<state::AbilityPickup> &pickups);

  private:
    std::unordered_map<state::EffectType, Texture2D> m_textures;
};

} // namespace Render
