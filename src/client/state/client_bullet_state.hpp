
#pragma once
#include "../../shared/state/bullet_state.hpp"
#include <stdint.h>

namespace state {

struct ClientBulletState : public BulletState {
    Vector2 serverPosition;
    Character::CharacterId characterId;
};

} // namespace state
