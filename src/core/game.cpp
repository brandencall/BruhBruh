#include "game.hpp"
#include "bullet_system.hpp"
#include "raylib.h"
#include "raymath.h"

Game::Game() : m_player(Player({10, 100})), m_camera(Camera2D()), m_bulletSystem(BulletSystem()) {
    InitWindow(1280, 720, "BruhBruh");
    SetTargetFPS(60);

    m_camera.target = m_player.GetPosition();
    m_camera.offset = {640, 360};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
}

Game::~Game() { CloseWindow(); }

void Game::Update() {
    Game::SetGameRunning(!WindowShouldClose());
    if (!m_running)
        return;

    float dt = GetFrameTime();

    m_player.Update(dt);
    m_camera.target = Vector2Lerp(m_camera.target, m_player.GetPosition(), 5.0f * dt);

    m_bulletSystem.Update(dt, m_player, m_camera);
    Game::Draw();
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode2D(m_camera);

    DrawDebugGrid();

    m_bulletSystem.Draw();
    m_player.Draw();

    EndMode2D();
    EndDrawing();
}

void Game::DrawDebugGrid() {
    const int gridSize = 64;
    const int gridCount = 50;

    for (int x = -gridCount; x <= gridCount; x++) {
        DrawLine(x * gridSize, -gridCount * gridSize, x * gridSize, gridCount * gridSize, Fade(LIGHTGRAY, 0.3f));
    }

    for (int y = -gridCount; y <= gridCount; y++) {
        DrawLine(-gridCount * gridSize, y * gridSize, gridCount * gridSize, y * gridSize, Fade(LIGHTGRAY, 0.3f));
    }

    // Origin marker
    DrawCircle(0, 0, 10, RED);
}

bool Game::GameRunning() { return Game::m_running; }

void Game::SetGameRunning(bool runningState) { Game::m_running = runningState; }
