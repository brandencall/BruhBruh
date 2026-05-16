#include "session_manager.hpp"
#include "network/ITransport.hpp"
#include "network/network_message_handler.hpp"
#include "network/steam_lobby_manager.hpp"
#include "network/steam_transport.hpp"
#include "scenes/game_scene.hpp"
#include "scenes/lobby_scene.hpp"
#include <iostream>
#include <memory>

SessionManager::SessionManager(SceneManager &sceneManager) : m_sceneManager(sceneManager) {}

void SessionManager::Initialize() {
    m_transport = new network::SteamTransport();
    m_handler = new NetworkMessageHandler();
    m_lobbyManager = std::make_unique<SteamLobbyManager>(dynamic_cast<network::SteamTransport &>(*m_transport));
}

void SessionManager::InitializeClientTransport(network::ITransport &transport, NetworkMessageHandler &handler) {
    m_transport = &transport;
    m_handler = &handler;
}

void SessionManager::Shutdown() {
    ShutdownServer();

    if (m_transport)
        m_transport->Shutdown();

    if (m_client)
        m_client->Disconnect();

    m_sceneManager.Clear();
    m_client.reset();
    m_server.reset();
    m_lobbyManager.reset();
    m_audioSystem.Unload();
    m_transport = nullptr;
}

network::ITransport *SessionManager::GetTransport() { return m_transport; }

void SessionManager::TickClient() {
    if (m_returningToStart) {
        m_lobbyManager->LeaveLobby();
        m_returningToStart = false;
        ShutdownServer();
        m_client.reset();
        m_server.reset();
        m_lobbyManager.reset();
        m_transport = nullptr;

        m_transport = new network::SteamTransport();
        m_lobbyManager = std::make_unique<SteamLobbyManager>(dynamic_cast<network::SteamTransport &>(*m_transport));
        return;
    }
    if (m_client) {
        m_client->Update();
    }
}

NetworkMessageHandler *SessionManager::GetHandler() { return m_handler; }

void SessionManager::HostGame(std::function<void()> onSuccess, std::function<void(const char *)> onError) {
    // Start the server thread first so it is ready by the time OnLobbyCreated fires
    StartServerThread();

    m_lobbyManager->SetCallbacks({
        .onLobbyCreated =
            [this, onSuccess = std::move(onSuccess)]() {
                // Wire the host player into the server, then signal it is ready to run
                m_server->AddHostToLobby(m_lobbyManager->GetLocalPlayerName());
                m_server->SignalReady();

                StartGameClient();
                // Tell the scene it can push LobbyScene
                if (onSuccess)
                    onSuccess();
            },
        .onError =
            [onError = std::move(onError)](const char *msg) {
                std::cerr << "Lobby error: " << msg << "\n";
                if (onError)
                    onError(msg);
            },
    });

    m_lobbyManager->CreateLobby(MAX_PLAYERS);
}

void SessionManager::JoinLobby(CSteamID lobbyId, std::function<void()> onSuccess,
                               std::function<void(const char *)> onError) {
    m_lobbyManager->SetCallbacks({
        .onLobbyJoined =
            [this, onSuccess = std::move(onSuccess)]() {
                StartGameClient();
                if (onSuccess)
                    onSuccess();
            },
        .onError =
            [onError = std::move(onError)](const char *msg) {
                std::cerr << "Lobby error: " << msg << "\n";
                if (onError)
                    onError(msg);
            },
    });

    m_lobbyManager->JoinLobby(lobbyId);
}

SteamLobbyManager *SessionManager::GetLobby() { return m_lobbyManager.get(); }

void SessionManager::CreateLobby() {
    m_sceneManager.Push(std::make_unique<LobbyScene>(*m_transport, *m_handler, *this));
}

void SessionManager::CreateGame(const state::LobbySlotState &currentPlayerState) {
    m_sceneManager.Replace(
        std::make_unique<GameScene>(*m_transport, *m_handler, *this, m_audioSystem, currentPlayerState));
}

void SessionManager::ReturnToStart() {
    m_returningToStart = true;
    m_sceneManager.RequestPop();
}

void SessionManager::StartServerThread() {
    if (!m_transport)
        return;

    m_server = std::make_unique<GameServer>();

    m_serverThread = std::thread([this]() {
        m_server->StartInProcess(*m_transport, *m_lobbyManager);
        m_server->RunServer();
    });
}

void SessionManager::StartGameClient() {
    m_client = std::make_unique<GameClient>(*m_transport, *m_handler);
    m_client->StartInProcess();
}

void SessionManager::ShutdownServer() {
    if (m_server)
        m_server->Stop();

    if (m_serverThread.joinable())
        m_serverThread.join();
}
