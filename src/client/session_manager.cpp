#include "session_manager.hpp"
#include "network/ITransport.hpp"
#include "network/steam_lobby_manager.hpp"
#include "network/steam_transport.hpp"
#include "scenes/game_scene.hpp"
#include "scenes/lobby_scene.hpp"
#include <iostream>
#include <memory>

SessionManager::SessionManager(SceneManager &sceneManager) : m_sceneManager(sceneManager) {}

void SessionManager::Initialize() {
    m_transport = std::make_unique<network::SteamTransport>();
    m_lobbyManager = std::make_unique<SteamLobbyManager>(*m_transport);
}

void SessionManager::Shutdown() {
    if (m_server)
        m_server->Stop();

    if (m_transport)
        m_transport->Shutdown();

    if (m_serverThread.joinable())
        m_serverThread.join();

    if (m_client)
        m_client->Disconnect();

    m_client.reset();
    m_server.reset();
    m_lobbyManager.reset();
    m_transport.reset();
}

network::ITransport &SessionManager::GetTransport() { return *m_transport; }

void SessionManager::TickClient() {
    if (m_client) {
        m_client->Update();
    }
}

NetworkMessageHandler &SessionManager::GetHandler() { return m_handler; }

void SessionManager::HostGame(std::function<void()> onSuccess, std::function<void(const char *)> onError) {
    // Start the server thread first so it is ready by the time OnLobbyCreated fires
    StartServerThread();

    m_lobbyManager->SetCallbacks({
        .onLobbyCreated =
            [this, onSuccess = std::move(onSuccess)]() {
                // Wire the host player into the server, then signal it is ready to run
                m_server->AddHostToLobby(m_lobbyManager->GetLocalPlayerName());
                m_server->SignalReady();

                // Create the client-side GameClient now that the server is up
                StartGameClient();

                // Tell the scene it can push LobbyScene
                if (onSuccess)
                    onSuccess();
            },
        .onMemberJoined = [](CSteamID who) { std::cout << SteamFriends()->GetFriendPersonaName(who) << " joined\n"; },
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
                std::cout << "In .onLobbyJoined callback" << std::endl;
                // m_server->AddClientToLobby(m_lobbyManager->GetLocalPlayerName());
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

SteamLobbyManager &SessionManager::GetLobby() { return *m_lobbyManager; }

void SessionManager::CreateLobby() {
    m_sceneManager.Push(std::make_unique<LobbyScene>(m_events, *m_transport, m_handler, *this));
}

void SessionManager::CreateGame(const state::LobbySlotState &currentPlayerState) {
    m_sceneManager.Replace(std::make_unique<GameScene>(m_events, *m_transport, m_handler, *this, currentPlayerState));
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
    m_client = std::make_unique<GameClient>(*m_transport, *m_lobbyManager, m_handler);
    m_client->StartInProcess();
}
