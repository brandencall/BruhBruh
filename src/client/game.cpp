#include "game.hpp"
#include "game_client.hpp"
#include "lobby_scene.hpp"
#include "raylib.h"
#include "scene_manager.hpp"
#include "scenes/game_scene.hpp"
#include "scenes/lobby_scene.hpp"
#include "scenes/start_scene.hpp"
#include <cassert>

void Game::Run() {
    CreateWindow();
    InitAudioDevice();
    m_session.Initialize();
    m_session.SetLobbyFactory([this]() { m_sceneManager.Push(std::make_unique<LobbyScene>(*this)); });
    m_session.SetGameSceneFactory([this](const state::LobbySlotState &currentPlayerState) {
        m_sceneManager.Replace(std::make_unique<GameScene>(*this, currentPlayerState));
    });

    // Push the start screen — it holds a ref to session and scenemanager
    m_sceneManager.Push(std::make_unique<StartScene>(*this));

    while (!WindowShouldClose() && !m_shouldQuit) {
        SteamAPI_RunCallbacks();
        assert(m_session.GetTransport() && "The m_session.GetTransport() assert failed...");
        m_session.GetTransport()->Pump();

        float dt = GetFrameTime();
        m_session.TickClient();
        m_sceneManager.Update(dt);
        m_sceneManager.Render();
    }

    m_session.Shutdown();
    m_audioSystem.Unload();
    CloseAudioDevice();
    CloseWindow();
}

void Game::RunLocal(GameClient &client) {
    CreateWindow();
    InitAudioDevice();
    m_session.InitializeClientTransport(*client.GetTransport(), *client.GetHandler());
    m_session.SetLobbyFactory([this]() { m_sceneManager.Push(std::make_unique<LobbyScene>(*this)); });
    m_session.SetGameSceneFactory([this](const state::LobbySlotState &currentPlayerState) {
        m_sceneManager.Replace(std::make_unique<GameScene>(*this, currentPlayerState));
    });

    m_session.CreateLobby();

    while (!WindowShouldClose() && !m_shouldQuit) {
        float dt = GetFrameTime();
        client.Update();
        m_sceneManager.Update(dt);
        m_sceneManager.Render();
    }

    m_audioSystem.Unload();
    CloseAudioDevice();
    CloseWindow();
}

void Game::RequestQuit() { m_shouldQuit = true; }

void Game::CreateWindow() {
    int monitor = GetCurrentMonitor(); // or pick manually later

    int width = GetMonitorWidth(monitor);
    int height = GetMonitorHeight(monitor);

    InitWindow(width, height, "BruhBruh");
    SetExitKey(KEY_NULL);

    // Borderless fullscreen
    SetWindowState(FLAG_WINDOW_UNDECORATED);

    Vector2 pos = GetMonitorPosition(monitor);
    SetWindowPosition((int)pos.x, (int)pos.y);

    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    SetWindowState(FLAG_VSYNC_HINT);

    int hz = GetMonitorRefreshRate(monitor);
    if (hz < 30 || hz > 360)
        hz = 60;

    SetTargetFPS(hz);
}

SessionManager *Game::GetSessionManager() { return &m_session; }

SceneManager *Game::GetSceneManager() { return &m_sceneManager; }

System::AudioSystem *Game::GetAudioSystem() { return &m_audioSystem; }

NetworkMessageHandler *Game::GetNetworkMessageHandler() { return m_session.GetHandler(); }

network::ITransport *Game::GetTransport() { return m_session.GetTransport(); }
