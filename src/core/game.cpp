#include "game.hpp"
#include "player.hpp"
#include "raymath.h"
#include "temp_entity.hpp"
#include "temp_wall.hpp"
#include <algorithm>
#include <memory>

Game::Game()
    : m_camera(Camera2D()), m_damageSystem(System::DamageSystem()), m_bulletSystem(),
      m_collisionSystem(System::CollisionSystem()) {
    InitWindow(1280, 720, "BruhBruh");
    SetTargetFPS(60);
    m_entities.emplace_back(std::make_unique<Player>(Vector2{10, 100}));
    m_player = static_cast<Player *>(m_entities.back().get());

    // TESTING
    m_entities.emplace_back(std::make_unique<Wall>(400.0f, 300.0f, 200.0f, 40.0f));
    m_wall = static_cast<Wall *>(m_entities.back().get());

    m_entities.emplace_back(std::make_unique<TestEntity>(200.0f, 200.0f, 40.0f, 40.0f));
    m_testEntity = static_cast<TestEntity *>(m_entities.back().get());
    //  ----------------------

    m_camera.target = m_player->GetPosition();
    m_camera.offset = {640, 360};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
    m_shootingCamera = m_camera;
}

Game::~Game() { CloseWindow(); }

void Game::Update() {
    Game::SetGameRunning(!WindowShouldClose());
    if (!m_running)
        return;

    float dt = GetFrameTime();

    m_player->Update(dt);
    m_shootingCamera.target = m_player->GetPosition();
    m_camera.target = Vector2Lerp(m_camera.target, m_player->GetPosition(), 5.0f * dt);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        auto bullet = m_bulletSystem.CreateBullet(*m_player, m_shootingCamera);
        m_entities.push_back(std::move(bullet));
    }
    m_bulletSystem.Update(dt, m_entities);
    m_collisionSystem.Update(m_entities);
    m_damageSystem.Update(m_entities);
    Game::Draw();
    RemoveDeadEntities();
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode2D(m_camera);

    DrawDebugGrid();

    // Draw all entities
    for (auto &entity : m_entities) {
        entity->Draw();
    }
    EndMode2D();
    EndDrawing();
}

void Game::RemoveDeadEntities() {
    m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                    [](const std::unique_ptr<Entity> &e) { return e->IsDead(); }),
                     m_entities.end());
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
