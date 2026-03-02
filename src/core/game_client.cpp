#include "game_client.hpp"
#include "../network/client.hpp"
#include "raylib.h"
#include <iostream>

GameClient::GameClient() : m_client(network::Client()) {
    InitWindow(1280, 720, "BruhBruh");
    SetTargetFPS(60);
}

// Send a disconnect signal to the server
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

    m_sendAccumulator += dt;

    if (m_sendAccumulator >= m_sendInterval) {
        auto input = CollectInput();
        m_client.Send(reinterpret_cast<const char *>(&input), sizeof(input));
        m_sendAccumulator -= m_sendInterval;
    }

    Render();
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
        // handle state later
        break;

    default:
        break;
    }
}
void GameClient::HandleJoinResponse(const char *buffer) {
    auto *response = (network::JoinResponsePacket *)buffer;
    m_playerId = response->playerId;
    m_joined = true;

    std::cout << "Assigned Player ID: " << m_playerId << "\n";
}

void GameClient::Render() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    EndDrawing();
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
