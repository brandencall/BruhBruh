#pragma once
#include "../shared/events.hpp"
#include "event_bus.hpp"
#include "events.hpp"

namespace Client {

struct EventHub {
    EventBus<event::PlayerDiedEvent> playerDied;
    EventBus<client::HitEvent> onHit;
};

} // namespace Client
