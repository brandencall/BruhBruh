#pragma once
#include "../client_transport.hpp"
#include "client_transport.hpp"
#include "event_hub.hpp"
#include "network/network_message_handler.hpp"
#include "scenes/game_scene.hpp"

class GameClient {
  public:
    ~GameClient();
    void Initialize();
    void Start(const char *ip, int port);

  private:
    void Update();
    void Connect(const char *ip, int port);
    void Disconnect();

    Client::EventHub m_events;
    network::ClientTransport m_transport;
    NetworkMessageHandler m_handler;
    GameScene m_gameScene{m_events, m_transport, m_handler};

    bool m_running = false;
};
