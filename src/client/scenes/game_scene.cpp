#include "game_scene.hpp"
#include "../../shared/characters/character_roster.hpp"
#include "../../shared/characters/character_types.hpp"
#include "../../shared/map/map_loader.hpp"
#include "../ui/screens/death_screen.hpp"
#include "../ui/screens/hud_screen.hpp"
#include "../ui/screens/scoreboard.hpp"
#include "raylib.h"

GameScene::GameScene(Client::EventHub &events, network::ClientTransport &transport, NetworkMessageHandler &handler,
                     SceneManager &sceneManager)
    : m_events(events), m_transport(transport), m_handler(handler), m_sceneManager(sceneManager) {}

void GameScene::OnEnter() {
    // Register packet handlers
    using PT = network::PacketType;
    m_handler.Register(PT::JoinResponse, [this](const char *b) { HandleJoinResponse(b); });
    m_handler.Register(PT::State, [this](const char *b) { HandleStateResponse(b); });
    m_handler.Register(PT::CurrentWorldState, [this](const char *b) { HandleCurrentWorldState(b); });
    m_handler.Register(PT::BulletSpawn, [this](const char *b) { HandleBulletSpawn(b); });
    m_handler.Register(PT::BulletDestroyed, [this](const char *b) { HandleBulletDestroyed(b); });
    m_handler.Register(PT::PlayerRespawned, [this](const char *b) { HandlePlayerRespawned(b); });
    m_handler.Register(PT::PlayerDamaged, [this](const char *b) { HandlePlayerDamaged(b); });
    m_handler.Register(PT::PlayerDied, [this](const char *b) { HandlePlayerDied(b); });
    m_handler.Register(PT::PlaceWall, [this](const char *b) { HandlePlaceWall(b); });
    m_handler.Register(PT::WallDamaged, [this](const char *b) { HandleWallDamaged(b); });
    m_handler.Register(PT::WallDestroyed, [this](const char *b) { HandleDestroyWall(b); });

    // Load map + assets
    Map::MapData mapData = Map::LoadMap(ACTIVE_MAP);
    m_worldState.m_map = mapData;
    m_worldState.m_tileMap = mapData.tileMap;
    m_tilemapRenderer.Load(ACTIVE_MAP.tileset);
    m_characterRender.Load();

    m_camera.offset = {std::round(1280 / 2.0f), std::round(720 / 2.0f)};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;

    Subscribe(m_events.playerDied, [this](const event::PlayerDiedEvent &e) {
        if (e.victim.id == m_worldState.m_currentPlayerId) {
            m_ui.Push(std::make_unique<UI::DeathScreen>(m_worldState.m_players[e.victim.id]));
        }
    });
}

void GameScene::OnExit() {
    // Unregister packet handlers
    using PT = network::PacketType;
    // TODO: Move Join Response to Lobby
    m_handler.Unregister(PT::JoinResponse);
    m_handler.Unregister(PT::State);
    m_handler.Unregister(PT::CurrentWorldState);
    m_handler.Unregister(PT::BulletSpawn);
    m_handler.Unregister(PT::BulletDestroyed);
    m_handler.Unregister(PT::PlayerRespawned);
    m_handler.Unregister(PT::PlayerDamaged);
    m_handler.Unregister(PT::PlayerDied);
    m_handler.Unregister(PT::PlaceWall);
    m_handler.Unregister(PT::WallDamaged);
    m_handler.Unregister(PT::WallDestroyed);

    m_characterRender.Unload();

    // Base Scene::OnExit unsubscribes any EventBus subscriptions
    Scene::OnExit();
}

uint32_t GameScene::GetCurrentPlayerId() const { return m_worldState.m_currentPlayerId; }

void GameScene::Update(float dt) {
    // TODO: Move Joining request to Lobby
    if (!m_joined) {
        m_joinRetryAccumulator += dt;
        if (m_joinRetryAccumulator >= 1.0f) {
            network::JoinPacket pkt{};
            pkt.header.type = network::PacketType::Join;
            m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
            m_joinRetryAccumulator = 0.0f;
        }
        return;
    }

    Sync(dt);
    m_bulletSystem.Update(dt);
    m_ui.Update(dt);
    HandleScoreboardInput();

    m_sendAccumulator += dt;
    if (m_sendAccumulator >= m_sendInterval) {
        if (!m_ui.BlocksGameInput()) {
            auto input = CollectInput();
            m_transport.send(network::PEER_SERVER, &input, sizeof(input));
        }
        m_sendAccumulator -= m_sendInterval;
    }
}

void GameScene::Sync(float dt) {
    // Sync players
    for (const auto &player : m_worldState.m_players) {
        if (!m_initialSnapDone && player.id == m_worldState.m_currentPlayerId)
            continue;
        m_characterRender.Sync(player, dt);
    }

    if (!m_joined)
        return;
    // Sync Camera

    if (m_worldState.m_currentPlayerId == -1)
        return;

    // TODO: First frame is off and looks terrible
    if (!m_initialSnapDone) {
        const auto &lp = m_worldState.m_players[m_worldState.m_currentPlayerId];
        if (lp.position.x == 0.0f && lp.position.y == 0.0f)
            return;

        m_characterRender.SnapToPosition(lp);
        Vector2 snappedPos = m_characterRender.GetPosition(m_worldState.m_currentPlayerId);
        m_camera.target = snappedPos;
        m_initialSnapDone = true;
        m_cameraReady = true;
        return;
    }

    Vector2 smoothedPos = m_characterRender.GetPosition(m_worldState.m_currentPlayerId);
    if (!m_cameraReady) {
        const auto &lp = m_worldState.m_players[m_worldState.m_currentPlayerId];
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

void GameScene::HandleScoreboardInput() {
    bool tabDown = IsKeyDown(KEY_TAB);
    bool scoreboardVisible = m_ui.HasScreenOfType<UI::Scoreboard>();

    if (tabDown && !scoreboardVisible) {
        m_ui.Push(std::make_unique<UI::Scoreboard>(m_worldState.m_players));
    }
}

void GameScene::HandleJoinResponse(const char *buffer) {
    auto *pkt = (network::JoinResponsePacket *)buffer;
    m_characterId = pkt->characterId;
    m_worldState.m_currentPlayerId = pkt->playerId;
    m_joined = true;
    m_ui.Push(
        std::make_unique<UI::HudScreen>(m_worldState.m_players[pkt->playerId], m_worldState.m_gameTime, m_events));
}

void GameScene::HandleStateResponse(const char *buffer) {
    auto *pkt = (network::StatePacket *)buffer;
    for (uint16_t i = 0; i < pkt->playerCount; ++i)
        m_worldState.m_players[pkt->players[i].id] = pkt->players[i];
    m_worldState.m_gameTime = pkt->currentGameTime;
}

void GameScene::HandleCurrentWorldState(const char *buffer) {
    auto *pkt = (network::CurrentWorldStatePacket *)buffer;

    std::unordered_map<Map::Vector2i, Map::DynamicWall, Map::GridHash> walls;
    walls.reserve(pkt->wallCount);
    for (uint16_t i = 0; i < pkt->wallCount; ++i)
        walls[pkt->walls[i].position] = pkt->walls[i].wall;
    m_wallManager.SetWalls(std::move(walls));

    for (uint16_t i = 0; i < pkt->playerCount; ++i)
        m_worldState.m_players[pkt->players[i].id] = pkt->players[i];
}

void GameScene::HandleBulletSpawn(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletSpawnPacket *>(buffer);
    Character::BulletDef bullet = Character::GetCharacterDef(pkt->characterId).bullet;
    m_bulletSystem.SpawnFromServerEvent(pkt->bulletId, pkt->ownerId, pkt->position, pkt->velocity, bullet);
}

void GameScene::HandleBulletDestroyed(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletDestroyedPacket *>(buffer);
    m_bulletSystem.Deactivate(pkt->bulletId);
}

void GameScene::HandlePlayerRespawned(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerRespawnedPacket *>(buffer);
    m_worldState.m_players[pkt->player.id] = pkt->player;
    m_characterRender.SnapToPosition(pkt->player);
    m_cameraReady = false;

    m_events.playerRespawned.Publish({pkt->player});
}

void GameScene::HandlePlayerDamaged(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerDamagedPacket *>(buffer);
    m_worldState.m_players[pkt->id].health = pkt->currentHealth;

    m_events.playerDamaged.Publish({pkt->id, pkt->currentHealth});
}

void GameScene::HandlePlayerDied(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerDiedPacket *>(buffer);
    m_worldState.m_players[pkt->victim.id] = pkt->victim;
    m_events.playerDied.Publish({pkt->victim, pkt->killer});
}

void GameScene::HandlePlaceWall(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlaceWallPacket *>(buffer);
    m_wallManager.PlaceWall(pkt->gridPos, pkt->maxHealth, pkt->player);
    m_worldState.m_players[pkt->player.id] = pkt->player;
}

void GameScene::HandleWallDamaged(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::WallDamagedPacket *>(buffer);
    m_wallManager.UpdateWallHealth(pkt->gridPos, pkt->currentHealth);
}

void GameScene::HandleDestroyWall(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::WallDestroyedPacket *>(buffer);
    m_wallManager.RemoveWall(pkt->gridPos, pkt->player.id);
    m_worldState.m_players[pkt->player.id] = pkt->player;
}

void GameScene::DrawMap(const Map::MapData &map) {
    for (const auto &wall : map.walls) {
        float w = wall.max.x - wall.min.x;
        float h = wall.max.y - wall.min.y;
        DrawRectangle(wall.min.x, wall.min.y, w, h, DARKBLUE);

        DrawRectangleLines(wall.min.x, wall.min.y, w, h, BLUE);
    }
}

void GameScene::Render() {

    if (!m_joined || !m_initialSnapDone) {
        RenderConnecting();
        return;
    }

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

void GameScene::RenderConnecting() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Connecting...", 560, 350, 20, WHITE);
    EndDrawing();
}

network::InputPacket GameScene::CollectInput() {
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
