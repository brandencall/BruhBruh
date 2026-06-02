#pragma once
#include "event_bus.hpp"
#include "events.hpp"

namespace Client {

struct EventHub {
    EventBus<client::PlayerDiedEvent> playerDied;
    EventBus<client::BulletDestroyedEvent> bulletDestroyed;
    EventBus<client::HitEvent> onHit;
    EventBus<client::WallPlacedEvent> onWallPlaced;
    EventBus<client::WallPickedUpEvent> onWallPickedUp;
    EventBus<event::WallInputDeniedEvent> onWallInputDenied;
    EventBus<client::GameStartingEvent> onGameStarting;
};

} // namespace Client
