#pragma once
#include "event_bus.hpp"
#include "events.hpp"

namespace Client {

struct EventHub {
    EventBus<client::PlayerDiedEvent> playerDied;
    EventBus<client::HitEvent> onHit;
};

} // namespace Client
