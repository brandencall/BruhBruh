#pragma once
#include "../../shared/state/lobby_slot_state.hpp"
#include "../client_transport.hpp"
#include "../event_hub.hpp"
#include "../network/network_message_handler.hpp"
#include "scene.hpp"

class LobbyScene : public Scene {
  public:
    LobbyScene(Client::EventHub &events, network::ClientTransport &transport, NetworkMessageHandler &handler,
               SceneManager &sceneManager);

    void OnEnter() override;
    void OnExit() override;
    void Update(float dt) override;
    void Render() override;

  private:
    // Packet handlers
    void HandleJoinResponse(const char *buf);
    void HandlePlayerJoined(const char *buf);
    void HandleLobbyState(const char *buf);
    void HandleGameStarting(const char *buf);

    void SendJoin();
    void SendReady();
    void RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y);

  private:
    bool m_joined = false;
    float m_joinRetryAccumulator = 0.0f;
    Client::EventHub &m_events;
    network::ClientTransport &m_transport;
    NetworkMessageHandler &m_handler;
    SceneManager &m_sceneManager;

    std::array<state::LobbySlotState, MAX_PLAYERS> m_players{};
    int m_localPlayerId = -1;
    bool m_ready = false;
};
