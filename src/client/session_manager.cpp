#include "session_manager.hpp"
#include "network/steam_lobby_manager.hpp"
#include "network/steam_transport.hpp"
#include "raylib.h"
#include "scenes/game_scene.hpp"
#include "scenes/lobby_scene.hpp"
#include <iostream>
#include <memory>

void SessionManager::Initialize() {
    m_transport = std::make_unique<network::SteamTransport>();

    m_lobbyManager = std::make_unique<SteamLobbyManager>(*m_transport);
    int monitor = GetCurrentMonitor(); // or pick manually later

    int width = GetMonitorWidth(monitor);
    int height = GetMonitorHeight(monitor);

    InitWindow(width, height, "BruhBruh");

    // Borderless fullscreen
    // SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    Vector2 pos = GetMonitorPosition(monitor);
    SetWindowPosition((int)pos.x, (int)pos.y);

    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    SetWindowState(FLAG_VSYNC_HINT);

    int hz = GetMonitorRefreshRate(monitor);
    if (hz < 30 || hz > 360)
        hz = 60;

    SetTargetFPS(hz);
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

    CloseWindow();
}

void SessionManager::StartHost() {
    StartGamerServerThread();
    StartGameClient();
    m_lobbyManager->SetCallbacks(
        {.onLobbyCreated =
             [this]() {
                 std::cout << "Lobby created\n";
                 m_server->AddHostToLobby(m_lobbyManager->GetLocalPlayerName());
                 m_server->SignalReady();
             },
         .onMemberJoined = [](CSteamID who) { std::cout << SteamFriends()->GetFriendPersonaName(who) << " joined\n"; },
         .onError = [](const char *msg) { std::cerr << "Lobby error: " << msg << "\n"; }});

    m_lobbyManager->CreateLobby(MAX_PLAYERS);
}

void SessionManager::StartClient() {
    m_lobbyManager->SetCallbacks(
        {.onLobbyJoined =
             [this]() {
                 std::cout << "Joined lobby\n";

                 m_client = std::make_unique<GameClient>(*m_transport, *m_lobbyManager, m_handler);
                 m_client->StartInProcess();

                 m_sceneManager.Push(std::make_unique<LobbyScene>(m_events, *m_transport, m_handler, *this));
             },
         .onError = [](const char *msg) { std::cerr << "Lobby error: " << msg << "\n"; }});
}

SteamLobbyManager &SessionManager::GetLobby() { return *m_lobbyManager; }

void SessionManager::CreateLobby() {
    m_sceneManager.Push(std::make_unique<LobbyScene>(m_events, *m_transport, m_handler, *this));
}

void SessionManager::CreateGame(const state::LobbySlotState &currentPlayerState) {
    m_sceneManager.Replace(std::make_unique<GameScene>(m_events, *m_transport, m_handler, *this, currentPlayerState));
}

void SessionManager::Run() {
    while (true) {
        SteamAPI_RunCallbacks();
        m_transport->Pump();

        if (WindowShouldClose())
            break;

        float dt = GetFrameTime();

        if (m_client)
            m_client->Update();

        m_sceneManager.Update(dt);
        m_sceneManager.Render();
    }
}

void SessionManager::StartGamerServerThread() {
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

    // push lobby scene
    m_sceneManager.Push(std::make_unique<LobbyScene>(m_events, *m_transport, m_handler, *this));
}
