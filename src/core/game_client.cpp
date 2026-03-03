#include "game_client.hpp"
#include "../network/client.hpp"
#include "raylib.h"
#include "raymath.h"
#include <iostream>

// This class will need to be split out once there are multiple scenes and not just the single Game scene
GameClient::GameClient() : m_client(network::Client()), m_worldState(ClientWorldState()), m_camera(Camera2D()) {
    InitWindow(1280, 720, "BruhBruh");
    SetTargetFPS(60);

    m_camera.offset = {640, 360};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
}

GameClient::~GameClient() {
    Disconnect();
    CloseWindow();
}

void GameClient::Connect(const char *ip, int port) { bool connected = m_client.Connect(ip, port); }

void GameClient::Disconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    packet.playerId = m_playerId;

    m_client.Send(reinterpret_cast<const char *>(&packet), static_cast<int>(sizeof(packet)));
}

void GameClient::SendJoin() {
    network::JoinPacket packet{};
    packet.header.type = network::PacketType::Join;
    m_client.Send(reinterpret_cast<const char *>(&packet), static_cast<int>(sizeof(packet)));
}

void GameClient::Update() {
    SetGameRunning(!WindowShouldClose());
    if (!m_running)
        return;
    float dt = GetFrameTime();

    Receive();
    Sync(dt);

    m_sendAccumulator += dt;

    if (m_sendAccumulator >= m_sendInterval) {
        auto input = CollectInput();
        m_client.Send(reinterpret_cast<const char *>(&input), sizeof(input));
        m_sendAccumulator -= m_sendInterval;
    }

    Render();
}

void GameClient::Sync(float dt) {
    // Sync players
    for (auto &[id, renderPlayer] : m_worldState.m_renderPlayers) {
        const auto &serverState = m_worldState.m_serverState[id];
        renderPlayer.Sync(serverState, dt);
    }
    // Sync Camera
    auto it = m_worldState.m_renderPlayers.find(m_worldState.m_currentPlayerId);
    if (it != m_worldState.m_renderPlayers.end()) {
        auto &player = it->second;
        m_camera.target = Vector2Lerp(m_camera.target, player.GetPosition(), 5.0f * dt);
    }
}

void GameClient::SetGameRunning(bool runningState) { m_running = runningState; }

void GameClient::Receive() {
    char buffer[1024];

    int bytes = m_client.Receive(buffer, sizeof(buffer));
    if (bytes <= 0)
        return;

    HandlePacket(buffer, bytes);
}

void GameClient::HandlePacket(char *buffer, size_t size) {
    network::PacketHeader *header = (network::PacketHeader *)buffer;

    switch (header->type) {
    case network::PacketType::JoinResponse:
        HandleJoinResponse(buffer);
        break;
    case network::PacketType::State:
        HandleStateResponse(buffer, size);
        break;
    default:
        break;
    }
}

void GameClient::HandleJoinResponse(const char *buffer) {
    auto *response = (network::JoinResponsePacket *)buffer;
    m_playerId = response->playerId;
    m_joined = true;
    m_worldState.m_currentPlayerId = response->playerId;
    std::cout << "Assigned Player ID: " << m_playerId << "\n";
}

void GameClient::HandleStateResponse(const char *buffer, size_t size) {
    if (size < sizeof(network::PacketHeader) + sizeof(uint32_t) + sizeof(uint16_t)) {
        std::cout << "State packet too small\n";
        return;
    }
    auto *response = (network::StatePacket *)buffer;
    size_t expectedSize = sizeof(network::PacketHeader) + sizeof(uint32_t) + sizeof(uint16_t) +
                          response->playerCount * sizeof(state::PlayerState);

    if (size < expectedSize) {
        std::cout << "Invalid state packet size\n";
        return;
    }

    for (uint16_t i = 0; i < response->playerCount; ++i) {
        const auto &player = response->players[i];
        m_worldState.m_serverState[player.id] = player;

        if (!m_worldState.m_renderPlayers.contains(player.id)) {
            m_worldState.m_renderPlayers.emplace(player.id, RenderPlayer(player.id));
        }
    }
}

void GameClient::Render() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    BeginMode2D(m_camera);

    DrawDebugGrid();

    for (auto &[id, renderPlayer] : m_worldState.m_renderPlayers) {
        renderPlayer.Draw();
    }

    EndMode2D();
    EndDrawing();
}

void GameClient::DrawDebugGrid() {
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

bool GameClient::GameRunning() { return m_running; }

network::InputPacket GameClient::CollectInput() {
    network::InputPacket packet{};
    packet.header.type = network::PacketType::Input;
    packet.playerId = m_playerId;

    float x = 0.0f;
    float y = 0.0f;

    if (IsKeyDown(KEY_W))
        y -= 1.0f;
    if (IsKeyDown(KEY_S))
        y += 1.0f;
    if (IsKeyDown(KEY_A))
        x -= 1.0f;
    if (IsKeyDown(KEY_D))
        x += 1.0f;

    packet.moveX = x;
    packet.moveY = y;

    uint8_t buttons = 0;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        buttons |= 1 << 0; // shoot

    packet.buttons = buttons;

    packet.sequence = m_inputSequence++;

    return packet;
}

void GameClient::SendInput(network::InputPacket &packet) {}
