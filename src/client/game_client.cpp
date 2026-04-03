#include "game_client.hpp"
#include "../shared/map/map_loader.hpp"
#include "characters/character_roster.hpp"
#include "characters/character_types.hpp"
#include "client_transport.hpp"
#include "hud_screen.hpp"
#include "raylib.h"
#include "scoreboard.hpp"
#include "state/player_state.hpp"
#include "ui/screens/death_screen.hpp"
#include <iostream>

// This class will need to be split out once there are multiple scenes and not just the single Game scene

GameClient::~GameClient() {
    Disconnect();
    m_characterRender.Unload();
    CloseWindow();
}

void GameClient::Initialize() {
    RegisterHandlers();
    InitWindow(1280, 720, "BruhBruh");
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    SetWindowState(FLAG_VSYNC_HINT);
    if (GetFPS() == 0) {
        int monitorHz = GetMonitorRefreshRate(GetCurrentMonitor());
        SetTargetFPS(monitorHz > 0 ? monitorHz : 60);
    }

    m_camera.offset = {std::round(1280 / 2.0f), std::round(720 / 2.0f)};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    Map::MapData mapData = Map::LoadMap(ACTIVE_MAP);
    m_worldState.m_map = mapData;             // collision, if client needs it
    m_worldState.m_tileMap = mapData.tileMap; // for the renderer
    m_tilemapRenderer.Load(ACTIVE_MAP.tileset);
    m_characterRender.Load();
}

void GameClient::RegisterHandlers() {
    m_handlers[network::PacketType::JoinResponse] = [this](const char *buf) { HandleJoinResponse(buf); };
    m_handlers[network::PacketType::State] = [this](const char *buf) { HandleStateResponse(buf); };
    m_handlers[network::PacketType::CurrentWorldState] = [this](const char *buf) { HandleCurrentWorldState(buf); };
    m_handlers[network::PacketType::BulletSpawn] = [this](const char *buf) { HandleBulletSpawn(buf); };
    m_handlers[network::PacketType::BulletDestroyed] = [this](const char *buf) { HandleBulletDestroyed(buf); };
    m_handlers[network::PacketType::PlayerRespawned] = [this](const char *buf) { HandlePlayerRespawned(buf); };
    m_handlers[network::PacketType::PlayerDamaged] = [this](const char *buf) { HandlePlayerDamaged(buf); };
    m_handlers[network::PacketType::PlayerDied] = [this](const char *buf) { HandlePlayerDied(buf); };
    m_handlers[network::PacketType::PlaceWall] = [this](const char *buf) { HandlePlaceWall(buf); };
    m_handlers[network::PacketType::WallDamaged] = [this](const char *buf) { HandleWallDamaged(buf); };
    m_handlers[network::PacketType::WallDestroyed] = [this](const char *buf) { HandleDestroyWall(buf); };
}

void GameClient::DrawMap(const Map::MapData &map) {
    for (const auto &wall : map.walls) {
        float w = wall.max.x - wall.min.x;
        float h = wall.max.y - wall.min.y;
        DrawRectangle(wall.min.x, wall.min.y, w, h, DARKBLUE);

        DrawRectangleLines(wall.min.x, wall.min.y, w, h, BLUE);
    }
}

void GameClient::Start(const char *ip, int port) {
    Connect(ip, port);
    m_running = true;
    while (m_running) {
        Update();
    }
}

void GameClient::Connect(const char *ip, int port) { m_transport.connect(ip, port); }

void GameClient::Disconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    packet.playerId = m_worldState.m_currentPlayerId;

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
        m_joinRetryAccumulator += dt;
        if (m_joinRetryAccumulator >= 1.0f) {
            SendJoin();
            m_joinRetryAccumulator = 0.0f;
        }
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Connecting...", 560, 350, 20, WHITE);
        EndDrawing();
        return;
    }

    Sync(dt);
    m_bulletSystem.Update(dt);
    m_killFeed.Update(dt);
    m_ui.Update(dt);

    m_sendAccumulator += dt;
    if (m_sendAccumulator >= m_sendInterval) {
        if (!m_ui.BlocksGameInput()) {
            auto input = CollectInput();
            m_transport.send(network::PEER_SERVER, &input, sizeof(input)); // ← transport
        }
        m_sendAccumulator -= m_sendInterval;
    }
    HandleScoreboardInput();

    Render();
}

void GameClient::HandleScoreboardInput() {
    bool tabDown = IsKeyDown(KEY_TAB);
    bool scoreboardVisible = m_ui.HasScreenOfType<UI::Scoreboard>();

    if (tabDown && !scoreboardVisible) {
        m_ui.Push(std::make_unique<UI::Scoreboard>(m_worldState.m_players));
    }
}

void GameClient::Sync(float dt) {
    // Sync players
    for (const auto &player : m_worldState.m_players) {
        m_characterRender.Sync(player, dt);
    }

    if (!m_joined)
        return;
    // Sync Camera

    if (m_worldState.m_currentPlayerId == -1)
        return;

    Vector2 smoothedPos = m_characterRender.GetPosition(m_worldState.m_currentPlayerId);
    if (!m_cameraReady) {
        m_characterRender.SnapToPosition(m_worldState.m_players[m_worldState.m_currentPlayerId]);
        smoothedPos = m_characterRender.GetPosition(m_worldState.m_currentPlayerId);
        m_camera.target = smoothedPos;
        m_cameraReady = true;
        return;
    }

    float smoothFactor = 1.0f - std::exp(-5.0f * dt);
    Vector2 smoothed = Vector2Lerp(m_camera.target, smoothedPos, smoothFactor);
    m_camera.target = smoothed;
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

    auto it = m_handlers.find(header->type);
    if (it != m_handlers.end()) {
        it->second(buffer);
    }
}

void GameClient::HandleJoinResponse(const char *buffer) {
    auto *response = (network::JoinResponsePacket *)buffer;
    m_characterId = response->characterId;
    m_worldState.m_currentPlayerId = response->playerId;
    m_ui.Push(std::make_unique<UI::HudScreen>(m_worldState.m_players[response->playerId], m_killFeed.GetFeed()));
    m_joined = true;
    m_cameraReady = false;
    std::cout << "Assigned Player ID: " << response->playerId << "\n";
    std::cout << "Assigned characterId: " << static_cast<int>(m_characterId) << "\n";
}

void GameClient::HandleStateResponse(const char *buffer) {
    auto *response = (network::StatePacket *)buffer;
    for (uint16_t i = 0; i < response->playerCount; ++i) {
        const auto &player = response->players[i];
        m_worldState.m_players[player.id] = player;
    }
}

void GameClient::HandleCurrentWorldState(const char *buffer) {
    auto *response = (network::CurrentWorldStatePacket *)buffer;

    std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> walls;
    walls.reserve(response->wallCount);
    for (uint16_t i = 0; i < response->wallCount; ++i) {
        walls[response->walls[i].position] = response->walls[i].wall;
    }
    m_wallManager.SetWalls(std::move(walls));

    for (uint16_t i = 0; i < response->playerCount; ++i) {
        const auto &player = response->players[i];
        m_worldState.m_players[player.id] = player;
    }
}

void GameClient::HandleBulletSpawn(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletSpawnPacket *>(buffer);

    Character::BulletDef bullet = Character::GetCharacterDef(pkt->characterId).bullet;
    m_bulletSystem.SpawnFromServerEvent(pkt->bulletId, pkt->ownerId, pkt->position, pkt->velocity, bullet);
}

void GameClient::HandleBulletDestroyed(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletDestroyedPacket *>(buffer);
    m_bulletSystem.Deactivate(pkt->bulletId);
}

void GameClient::HandlePlayerRespawned(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerRespawnedPacket *>(buffer);
    m_worldState.m_players[pkt->player.id] = pkt->player;
    m_characterRender.SnapToPosition(pkt->player);
    m_cameraReady = false;
}

void GameClient::HandlePlayerDamaged(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerDamagedPacket *>(buffer);
    m_worldState.m_players[pkt->id].health = pkt->currentHealth;
}

void GameClient::HandlePlayerDied(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerDiedPacket *>(buffer);
    if (pkt->victim.id == m_worldState.m_currentPlayerId) {
        const state::PlayerState &currentPlayer = m_worldState.m_players[pkt->victim.id];
        m_ui.Push(std::make_unique<UI::DeathScreen>(currentPlayer));
    }
    // TODO: spawn death effect
    m_worldState.m_players[pkt->victim.id] = pkt->victim;
    m_killFeed.Push(pkt->killer.name, pkt->victim.name);
}

void GameClient::HandlePlaceWall(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlaceWallPacket *>(buffer);
    m_wallManager.PlaceWall(pkt->gridPos, pkt->maxHealth, pkt->player);
    m_worldState.m_players[pkt->player.id] = pkt->player;
}

void GameClient::HandleWallDamaged(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::WallDamagedPacket *>(buffer);
    m_wallManager.UpdateWallHealth(pkt->gridPos, pkt->currentHealth);
}

void GameClient::HandleDestroyWall(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::WallDestroyedPacket *>(buffer);
    m_wallManager.RemoveWall(pkt->gridPos, pkt->player.id);
    m_worldState.m_players[pkt->player.id] = pkt->player;
}

void GameClient::Render() {
    BeginDrawing();

    ClearBackground(DARKGRAY);
    Camera2D renderCam = m_camera;
    renderCam.target = {std::round(m_camera.target.x), std::round(m_camera.target.y)};
    renderCam.offset = {std::round(m_camera.offset.x), std::round(m_camera.offset.y)};
    BeginMode2D(renderCam);

    DrawMap(m_worldState.m_map);
    m_tilemapRenderer.Draw(m_worldState.m_tileMap);

    for (const auto &player : m_worldState.m_players) {
        if (player.respawnTimer > 0.0f)
            continue;

        m_characterRender.Draw(player);

        // Use lerped position so hurtbox stays on the sprite
        Vector2 renderPos = m_characterRender.GetPosition(player.id);
        Vector2 hurtboxCenter = {renderPos.x + player.hurtbox.offsetX, renderPos.y + player.hurtbox.offsetY};
        DrawCircleV(hurtboxCenter, player.hurtbox.radius, {255, 0, 0, 80});
        DrawCircleLinesV(hurtboxCenter, player.hurtbox.radius, RED);
    }

    for (const auto &bullet : m_bulletSystem.GetBullets()) {
        if (!bullet.active)
            continue;
        DrawCircleV(bullet.hitbox.circle.center, 4.0f, YELLOW);
    }
    for (const auto &[gridPos, wall] : m_wallManager.GetAllWalls()) {
        if (!wall.active)
            continue;

        float x = gridPos.x * Map::GRID_CELL_SIZE;
        float y = gridPos.y * Map::GRID_CELL_SIZE;

        DrawRectangle(x, y, Map::GRID_CELL_SIZE, Map::GRID_CELL_SIZE, BROWN);
        DrawRectangleLines(x, y, Map::GRID_CELL_SIZE, Map::GRID_CELL_SIZE, DARKBROWN);

        // Health bar
        // float healthPct = wall.health / wall.maxHealth;
        // DrawRectangle(x, y - 8, Map::GRID_CELL_SIZE * healthPct, 4, GREEN);
    }

    EndMode2D();

    m_ui.Render();

    int fps = (int)(1.0f / GetFrameTime());
    std::string fpsText = std::to_string(fps) + " fps";
    int textWidth = MeasureText(fpsText.c_str(), 20);
    DrawText(fpsText.c_str(), 1280 - textWidth - 10, 10, 20, GREEN);

    EndDrawing();
}

network::InputPacket GameClient::CollectInput() {
    network::InputPacket packet{};
    if (!m_joined)
        return packet;

    packet.header.type = network::PacketType::Input;
    packet.playerId = m_worldState.m_currentPlayerId;
    packet.characterId = m_characterId;

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
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        buttons |= 1 << 1; // place_wall

    packet.buttons = buttons;
    packet.sequence = m_inputSequence++;

    // Aim direction
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), m_camera);

    if (m_worldState.m_currentPlayerId != -1) {
        // Use server position for aim calculation to match where server will spawn bullet
        const state::PlayerState &currPlayer = m_worldState.m_players[m_worldState.m_currentPlayerId];
        Vector2 playerPos = {currPlayer.position.x, currPlayer.position.y};

        if (buttons & (1 << 0)) {
            // Send aim direction for shooting
            Vector2 aimDir = Vector2Subtract(mouseWorld, playerPos);
            packet.aimX = aimDir.x;
            packet.aimY = aimDir.y;
        } else if (buttons & (1 << 1)) {
            // Send raw world position for wall placement
            packet.aimX = mouseWorld.x;
            packet.aimY = mouseWorld.y;
        }
    }

    m_lastButtons = buttons;

    return packet;
}

void GameClient::SendInput(network::InputPacket &packet) {
    m_transport.send(network::PEER_SERVER, &packet, sizeof(packet));
}
