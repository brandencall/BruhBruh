#include "game.hpp"
#include "raylib.h"
#include "scenes/start_scene.hpp"

void Game::Run() {
    CreateWindow();
    InitAudioDevice();
    m_session.Initialize();

    // Push the start screen — it holds a ref to session and scenemanager
    m_sceneManager.Push(std::make_unique<StartScene>(*this, m_session, m_sceneManager));

    while (!WindowShouldClose() && !m_shouldQuit) {
        SteamAPI_RunCallbacks();
        m_session.GetTransport().Pump();

        float dt = GetFrameTime();
        m_session.TickClient();
        m_sceneManager.Update(dt);
        m_sceneManager.Render();
    }

    m_session.Shutdown();
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
