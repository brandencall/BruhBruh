
#pragma once
#include "../../shared/state/bullet_state.hpp"
#include "raylib.h"
#include <stdint.h>

namespace state {

struct ClientBulletState : public BulletState {
    Vector2 serverPosition;
};

} // namespace state
