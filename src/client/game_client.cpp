#include "game_client.hpp"
#include "../shared/map/map_loader.hpp"
#include "client_transport.hpp"
#include "raylib.h"
#include "raymath.h"
#include <iostream>

// This class will need to be split out once there are multiple scenes and not just the single Game scene

GameClient::~GameClient() {
    Disconnect();
    CloseWindow();
}

void GameClient::Initialize() {
    InitWindow(1280, 720, "BruhBruh");
    SetTargetFPS(60);

    m_camera.offset = {640, 360};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    m_worldState.m_map = LoadMap(MAP_PATH);
}

void GameClient::DrawMap(const MapData &map) {
    for (const auto &wall : map.walls) {
        float w = wall.max.x - wall.min.x;
        float h = wall.max.y - wall.min.y;
        DrawRectangle(wall.min.x, wall.min.y, w, h, DARKBLUE);

        DrawRectangleLines(wall.min.x, wall.min.y, w, h, BLUE);
    }
}

void GameClient::Start(const char *ip, int port) {
    Connect(ip, port);
    SendJoin();
    while (m_running) {
        Update();
    }
}

void GameClient::Connect(const char *ip, int port) { m_transport.connect(ip, port); }

void GameClient::Disconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    packet.playerId = m_playerId;

    m_transport.send(network::PEER_SERVER, &packet, sizeof(packet));
}

void GameClient::SendJoin() {
    network::JoinPacket packet{};
    packet.header.type = network::PacketType::Join;
    m_transport.send(network::PEER_SERVER, &packet, sizeof(packet));
}

void GameClient::Update() {
    SetGameRunning(!WindowShouldClose());
    if (!m_running)
        return;
    float dt = GetFrameTime();

    Receive();

    if (!m_joined) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Connecting...", 560, 350, 20, WHITE);
        EndDrawing();
        return;
    }

    Sync(dt);
    m_bulletSystem.Update(dt, m_worldState.m_players);

    m_sendAccumulator += dt;
    if (m_sendAccumulator >= m_sendInterval) {
        auto input = CollectInput();
        m_transport.send(network::PEER_SERVER, &input, sizeof(input)); // ← transport
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
        m_camera.target = Vector2Lerp(m_camera.target, it->second.GetPosition(), 5.0f * dt);
    }
}

void GameClient::SetGameRunning(bool runningState) { m_running = runningState; }

void GameClient::Receive() {
    network::InboundPacket pkt;
    while (m_transport.recv(pkt)) {
        HandlePacket(pkt.data, pkt.size);
    }
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
    case network::PacketType::BulletSpawn:
        HandleBulletSpawn(buffer);
        break;
    case network::PacketType::BulletHit:
        HandleBulletHit(buffer);
        break;
    case network::PacketType::BulletExpired:
        HandleBulletExpired(buffer);
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
    for (uint16_t i = 0; i < response->playerCount; ++i) {
        const auto &player = response->players[i];
        m_worldState.m_serverState[player.id] = player;
        if (!m_worldState.m_renderPlayers.contains(player.id)) {
            m_worldState.m_renderPlayers.emplace(player.id, RenderPlayer(player.id));
        }
    }
}

void GameClient::HandleBulletSpawn(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletSpawnPacket *>(buffer);

    m_bulletSystem.SpawnWithId(pkt->bulletId, pkt->ownerId, pkt->position, pkt->velocity, pkt->lifetime);
}

void GameClient::HandleBulletHit(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletHitPacket *>(buffer);
    m_bulletSystem.Deactivate(pkt->bulletId);
    // TODO: spawn hit effect at pkt->hitPosition
}

void GameClient::HandleBulletExpired(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletExpirePacket *>(buffer);
    m_bulletSystem.Deactivate(pkt->bulletId);
}

void GameClient::Render() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    BeginMode2D(m_camera);

    DrawDebugGrid();
    DrawMap(m_worldState.m_map);

    for (auto &[id, renderPlayer] : m_worldState.m_renderPlayers) {
        renderPlayer.Draw();
        Vector2 pos = renderPlayer.GetPosition(); // use lerped position, not server state
        const auto &state = m_worldState.m_serverState[id];
        Vector2 hurtboxCenter = {pos.x + state.hurtbox.offsetX, pos.y + state.hurtbox.offsetY};
        DrawCircleV(hurtboxCenter, state.hurtbox.radius, {255, 0, 0, 80});
        DrawCircleLinesV(hurtboxCenter, state.hurtbox.radius, RED);
    }

    for (const auto &bullet : m_bulletSystem.GetBullets()) {
        if (!bullet.active)
            continue;
        DrawCircleV(bullet.hitbox.circle.center, 4.0f, YELLOW);
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

network::InputPacket GameClient::CollectInput() {
    network::InputPacket packet{};
    if (!m_joined)
        return packet;

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

    // Aim direction
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), m_camera);
    auto it = m_worldState.m_renderPlayers.find(m_playerId);
    if (it != m_worldState.m_renderPlayers.end()) {
        Vector2 playerPos = it->second.GetPosition();
        Vector2 aimDir = Vector2Subtract(mouseWorld, playerPos);

        packet.aimX = aimDir.x;
        packet.aimY = aimDir.y;

        // Client-side prediction: spawn bullet immediately on click
        bool shootNow = buttons & (1 << 0);
        bool shootPrev = m_lastButtons & (1 << 0);
    }

    m_lastButtons = buttons;

    return packet;
}

void GameClient::SendInput(network::InputPacket &packet) {
    m_transport.send(network::PEER_SERVER, &packet, sizeof(packet));
}
