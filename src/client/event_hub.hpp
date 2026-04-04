#pragma once
#include "../shared/events.hpp"
#include "event_bus.hpp"

namespace Client {

struct EventHub {
    EventBus<event::PlayerDiedEvent> playerDied;
    EventBus<event::PlayerDamagedEvent> playerDamaged;
    EventBus<event::PlayerRespawnEvent> playerRespawned;
};

} // namespace Client
