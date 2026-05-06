#pragma once
#include "../client_transport.hpp"
#include "../shared/network/ITransport.hpp"
#include "client_transport.hpp"
#include "network/network_message_handler.hpp"
#include "scenes/scene_manager.hpp"

class GameClient {
  public:
    GameClient(network::ITransport &transport, NetworkMessageHandler &handler);

    GameClient(NetworkMessageHandler &handler);

    void Start(const char *ip, int port);
    void StartInProcess();
    void Update();
    void Disconnect();
    network::ITransport *GetTransport();
    NetworkMessageHandler *GetHandler();

  private:
    network::ClientTransport m_ownedTransport;
    network::ITransport *m_transport = nullptr;
    NetworkMessageHandler &m_handler;
    SceneManager m_sceneManager;

    bool m_running = false;
};
