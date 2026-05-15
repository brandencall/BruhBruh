#include "game_scene.hpp"
#include "../../shared/characters/character_movement.hpp"
#include "../../shared/characters/character_roster.hpp"
#include "../../shared/characters/character_types.hpp"
#include "../../shared/map/map_loader.hpp"
#include "../ui/screens/confirm_quit_screen.hpp"
#include "../ui/screens/death_screen.hpp"
#include "../ui/screens/game_end_screen.hpp"
#include "../ui/screens/hud_screen.hpp"
#include "../ui/screens/scoreboard.hpp"
#include "raylib.h"
#include <cstdint>
#include <iostream>

GameScene::GameScene(network::ITransport &transport, NetworkMessageHandler &handler, SessionManager &sessionManager,
                     state::LobbySlotState currentPlayerState)
    : m_transport(transport), m_handler(handler), m_sessionManager(sessionManager),
      m_currentPlayerId(currentPlayerState.id), m_currenCharacterId(currentPlayerState.characterId) {}

void GameScene::OnEnter() {
    HideCursor();

    // Register packet handlers
    using PT = network::PacketType;
    m_handler.Register(PT::GameBegin, [this](const char *b) { HandleGameBegin(b); });
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
    m_handler.Register(PT::GameEnd, [this](const char *b) { HandleGameEnd(b); });
    m_handler.Register(PT::SwitchToLobby, [this](const char *b) { HandleSwitchToLobby(b); });
    m_handler.Register(PT::HostDisconnected, [this](const char *b) { HandleHostDisconnected(b); });

    // Load map + assets
    Map::MapData mapData = Map::LoadMap(ACTIVE_MAP);
    m_worldState.m_map = mapData;
    m_worldState.m_tileMap = mapData.tileMap;
    m_worldState.m_currentPlayerId = m_currentPlayerId;
    m_tilemapRenderer.Load(ACTIVE_MAP.tileset);
    // TODO: Optimize loading so that we don't load all textures and just the ones that are needed
    m_characterRender.Load();
    m_wallRender.Load();
    m_bulletSystem.Load();
    m_bulletSystem.SetMap(m_worldState.m_map);

    m_camera.Init(m_events.onHit, m_events.playerDied, m_events.onWallPlaced);

    m_audioAvailable = IsAudioDeviceReady();

    Subscribe(m_events.playerDied, [this](const client::PlayerDiedEvent &e) {
        if (e.data.victim.id == m_worldState.m_currentPlayerId) {
            m_ui.Push(std::make_unique<UI::DeathScreen>(m_worldState.m_players[e.data.victim.id]));
        }
    });
}

void GameScene::OnExit() {
    std::cout << "GameScene::CleanUp()" << std::endl;
    // Unregister packet handlers
    using PT = network::PacketType;
    m_handler.Unregister(PT::GameBegin);
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
    m_handler.Unregister(PT::GameEnd);
    m_handler.Unregister(PT::SwitchToLobby);
    m_handler.Unregister(PT::HostDisconnected);

    m_tilemapRenderer.Unload();
    m_characterRender.Unload();
    m_wallRender.Unload();
    m_bulletSystem.Unload();
    m_audioSystem.Unload();
    m_ui.Clear();

    ShowCursor();

    Scene::OnExit();
}

uint32_t GameScene::GetCurrentPlayerId() const { return m_worldState.m_currentPlayerId; }

void GameScene::Update(float dt) {
    Sync(dt);
    m_bulletSystem.Update(dt, m_worldState.m_players, m_wallManager.GetAllWalls());
    m_ui.Update(dt);
    m_camera.Update(dt);
    if (m_ui.BlocksGameInput())
        return;

    HandleScoreboardInput();

    if (IsKeyPressed(KEY_ESCAPE))
        m_ui.Push(std::make_unique<UI::ConfirmQuitScreen>([this]() { m_sessionManager.ReturnToStart(); }));

    TickPrediction(dt);
}

void GameScene::Sync(float dt) {
    // Interpolate remote players toward their last known server position
    for (const auto &player : m_worldState.m_players) {
        if (!player.active || player.id == m_worldState.m_currentPlayerId)
            continue;
        m_characterRender.Sync(player, dt);
    }

    if (!m_joined || m_worldState.m_currentPlayerId == -1)
        return;

    // Wait until the local player has a real position before starting
    if (!m_initialSnapDone) {
        const auto &lp = m_worldState.m_players[m_worldState.m_currentPlayerId];
        if (lp.position.x == 0.0f && lp.position.y == 0.0f)
            return;

        m_camera.SetPosition(lp.position);
        m_initialSnapDone = true;
    }

    // Smoothly follow the renderer position (which tracks predicted pos for local player)
    Vector2 rendererPos = m_characterRender.GetPosition(m_worldState.m_currentPlayerId);
    float smoothFactor = 1.0f - std::exp(-15.0f * dt);
    Vector2 smoothed = Vector2Lerp(m_camera.GetCamera()->target, rendererPos, smoothFactor);
    m_camera.SetPosition({std::round(smoothed.x), std::round(smoothed.y)});
}

void GameScene::HandleScoreboardInput() {
    bool tabDown = IsKeyDown(KEY_TAB);
    bool scoreboardVisible = m_ui.HasScreenOfType<UI::Scoreboard>();

    if (tabDown && !scoreboardVisible) {
        m_ui.Push(std::make_unique<UI::Scoreboard>(m_worldState.m_players));
    }
}

void GameScene::HandleGameBegin(const char *buffer) {
    auto *pkt = (network::GameBeginPacket *)buffer;
    m_gameBeginTimer = pkt->countdown;

    if (!m_joined) {
        m_worldState.m_currentPlayerId = m_currentPlayerId;
        m_worldState.m_gameTime = pkt->gameTime;
        m_joined = true;

        for (uint16_t i = 0; i < pkt->playerCount; ++i)
            m_worldState.m_players[pkt->players[i].id] = pkt->players[i];

        const state::PlayerState &lp = m_worldState.m_players[m_currentPlayerId];
        m_predictedPos = lp.position;
        m_smoothedPredictedPos = lp.position;
        m_predictionInitialised = true;

        m_characterRender.SnapToPosition(lp);
        m_camera.SetPosition(lp.position);
        m_initialSnapDone = true;

        m_ui.Push(std::make_unique<UI::HudScreen>(m_worldState.m_players[m_currentPlayerId], m_worldState.m_gameTime,
                                                  m_events));
        if (m_audioAvailable)
            m_audioSystem.Init(m_events.onHit, m_events.playerDied);
    }
}

void GameScene::HandleStateResponse(const char *buffer) {
    auto *pkt = (network::StatePacket *)buffer;

    uint32_t ackedSeq = pkt->players[m_currentPlayerId].currentInput.sequence;
    if (ackedSeq <= m_lastAckedSeq)
        return;

    for (uint16_t i = 0; i < pkt->playerCount; ++i)
        m_worldState.m_players[pkt->players[i].id] = pkt->players[i];

    m_worldState.m_gameTime = pkt->currentGameTime;

    if (!m_predictionInitialised)
        return;

    m_lastAckedSeq = ackedSeq;
    const state::PlayerState &lp = m_worldState.m_players[m_currentPlayerId];
    Reconcile(lp.position, ackedSeq);
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
    Character::CharacterDef character = Character::GetCharacterDef(pkt->characterId);
    if (pkt->ownerId == m_currentPlayerId) {
        m_bulletSystem.ResolveLocalPredictedBullet(*pkt, m_currentPlayerId);
    } else {
        m_bulletSystem.SpawnFromServerEvent(*pkt);
    }
}

void GameScene::HandleBulletDestroyed(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::BulletDestroyedPacket *>(buffer);
    m_bulletSystem.Deactivate(pkt->bulletId, pkt->position);
}

void GameScene::HandlePlayerRespawned(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerRespawnedPacket *>(buffer);
    m_worldState.m_players[pkt->player.id] = pkt->player;
    m_characterRender.SnapToPosition(pkt->player);

    // only snap camera for the local player
    if (pkt->player.id == m_worldState.m_currentPlayerId) {
        m_predictedPos = pkt->player.position;
        m_smoothedPredictedPos = pkt->player.position;

        m_inputBuffer = {};
        m_lastAckedSeq = 0;

        m_camera.SetPosition(pkt->player.position);
    }
}

void GameScene::HandlePlayerDamaged(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerDamagedPacket *>(buffer);
    m_worldState.m_players[pkt->vitimId].health = pkt->currentHealth;

    m_events.onHit.Publish(
        {pkt->attackerId, pkt->vitimId, m_worldState.m_players[pkt->attackerId].characterId, m_currentPlayerId});
}

void GameScene::HandlePlayerDied(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlayerDiedPacket *>(buffer);
    m_worldState.m_players[pkt->victim.id] = pkt->victim;
    m_events.onHit.Publish(
        {pkt->killer.id, pkt->victim.id, m_worldState.m_players[pkt->killer.id].characterId, m_currentPlayerId});
    m_events.playerDied.Publish({pkt->victim, pkt->killer, m_worldState.m_players[m_currentPlayerId]});
}

void GameScene::HandlePlaceWall(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::PlaceWallPacket *>(buffer);
    m_wallManager.PlaceWall(pkt->gridPos, pkt->maxHealth, pkt->player, (float)GetTime());
    m_worldState.m_players[pkt->player.id] = pkt->player;
    m_events.onWallPlaced.Publish({pkt->player.id, pkt->gridPos, m_currentPlayerId});
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

void GameScene::HandleGameEnd(const char *buffer) {
    auto *pkt = reinterpret_cast<const network::GameEndPacket *>(buffer);
    if (!m_gameEndScreenActive) {
        m_gameEndScreenActive = true;
        UI::GameEndData data;
        data.countdown = pkt->countdown;
        data.playerCount = pkt->playerCount;
        std::copy(pkt->rankedPlayers, pkt->rankedPlayers + pkt->playerCount, data.rankings.begin());
        m_ui.Push(std::make_unique<UI::GameEndScreen>(std::move(data)));
    }

    if (auto *screen = m_ui.Get<UI::GameEndScreen>()) {
        screen->UpdateCountdown(pkt->countdown);
    }
}

void GameScene::HandleSwitchToLobby(const char *buf) { m_sessionManager.CreateLobby(); }

void GameScene::HandleHostDisconnected(const char *buf) {
    std::cout << "Calling m_sessionManager.ReturnToStart() from GameScene::HandleHostDisconnected()" << std::endl;
    m_sessionManager.ReturnToStart();
}

void GameScene::DrawMap(const Map::MapData &map) {
    for (const auto &wall : map.walls) {
        Rectangle rect = {wall.min.x, wall.min.y, wall.max.x - wall.min.x, wall.max.y - wall.min.y};
        DrawRectangleRec(rect, DARKBLUE);
        DrawRectangleLinesEx(rect, 1.0f, BLUE);
    }
}

void GameScene::Render() {
    if (!m_joined || !m_initialSnapDone) {
        RenderConnecting();
        return;
    }

    BeginDrawing();

    ClearBackground(DARKGRAY);
    BeginMode2D(*m_camera.GetCamera());

    DrawMap(m_worldState.m_map);

    m_tilemapRenderer.Draw(m_worldState.m_tileMap);
    m_characterRender.Draw(m_worldState.m_players);
    m_bulletSystem.Draw();
    m_wallRender.Draw(m_wallManager.GetAllWalls());

    EndMode2D();

    m_ui.Render();

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    if (m_gameBeginTimer > 0) {
        const char *countdownText = TextFormat("%d", (int)std::ceil(m_gameBeginTimer));
        int fontSize = 128;
        int textWidth = MeasureText(countdownText, fontSize);
        DrawText(countdownText, (screenW - textWidth) / 2, (screenH / 2) - (fontSize / 2), fontSize, YELLOW);
    }

    int fps = (int)(1.0f / GetFrameTime());
    std::string fpsText = std::to_string(fps) + " fps";
    int textWidth = MeasureText(fpsText.c_str(), 20);
    DrawText(fpsText.c_str(), screenW - textWidth - 10, 10, 20, GREEN);

    RenderCursor();
    EndDrawing();
}

void GameScene::RenderConnecting() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Connecting...", 560, 350, 20, WHITE);
    EndDrawing();
}

void GameScene::RenderCursor() {
    Vector2 mouse = GetMousePosition();
    float shootCooldownTimer = m_worldState.m_players[m_currentPlayerId].shootTimer;
    float shootCooldown = Character::GetCharacterDef(m_currenCharacterId).bullet.cooldown;
    float t = 1.0f - (shootCooldownTimer / shootCooldown); // 1 = ready, 0 = just shot
    bool ready = t >= 1.0f;

    // Crosshair lines
    float gap = 4.0f;
    float len = 8.0f;
    Color crossColor = ready ? RAYWHITE : Fade(RAYWHITE, 0.4f);

    DrawLineV({mouse.x - gap - len, mouse.y}, {mouse.x - gap, mouse.y}, crossColor);
    DrawLineV({mouse.x + gap, mouse.y}, {mouse.x + gap + len, mouse.y}, crossColor);
    DrawLineV({mouse.x, mouse.y - gap - len}, {mouse.x, mouse.y - gap}, crossColor);
    DrawLineV({mouse.x, mouse.y + gap}, {mouse.x, mouse.y + gap + len}, crossColor);

    // Cooldown ring — only visible when on cooldown
    if (!ready) {
        float radius = 14.0f;
        DrawCircleLines((int)mouse.x, (int)mouse.y, radius, Fade(RAYWHITE, 0.15f));
        DrawRing(mouse, radius - 2.0f, radius, -90.0f, -90.0f + (360.0f * t), 32, SKYBLUE);
    }
}

void GameScene::TickPrediction(float dt) {
    if (!m_joined || m_gameBeginTimer > 0.0f)
        return;

    const state::PlayerState &lp = m_worldState.m_players[m_worldState.m_currentPlayerId];
    if (lp.state == state::State::Dead)
        return;

    float moveX = 0.0f, moveY = 0.0f;

    if (IsKeyDown(KEY_W))
        moveY -= 1.0f;
    if (IsKeyDown(KEY_S))
        moveY += 1.0f;
    if (IsKeyDown(KEY_A))
        moveX -= 1.0f;
    if (IsKeyDown(KEY_D))
        moveX += 1.0f;

    PredictLocalActions();

    state::PlayerState predicted = lp;
    predicted.position = m_predictedPos;
    predicted.currentInput.moveX = moveX;
    predicted.currentInput.moveY = moveY;

    std::vector<Collision::AABB> dynamicColliders = m_wallManager.GetColliders();

    Character::SimulateMove(predicted, dt, m_worldState.m_map.walls, dynamicColliders);

    m_predictedPos = predicted.position;

    m_sendAccumulator += dt;

    if (m_sendAccumulator >= m_sendInterval) {
        network::InputPacket pkt = BuildInputPacket();

        pkt.moveX = moveX;
        pkt.moveY = moveY;

        m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));

        size_t slot = pkt.sequence % INPUT_BUFFER_SIZE;
        m_inputBuffer[slot] = {pkt, dt};

        m_sendAccumulator -= m_sendInterval;
    }

    if (!m_smoothedPredictedPos.x && !m_smoothedPredictedPos.y)
        m_smoothedPredictedPos = m_predictedPos;

    float dist = Vector2Distance(m_smoothedPredictedPos, m_predictedPos);

    if (dist > SNAP_THRESHOLD) {
        m_smoothedPredictedPos = m_predictedPos;
    } else {
        float t = 1.0f - std::exp(-20.0f * dt);
        m_smoothedPredictedPos = Vector2Lerp(m_smoothedPredictedPos, m_predictedPos, t);
    }

    state::PlayerState fakeLp = lp;
    fakeLp.position = m_smoothedPredictedPos;

    m_characterRender.SnapToPosition(fakeLp);
}

void GameScene::PredictLocalActions() {
    uint8_t buttons = 0;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        buttons |= 1 << 0;

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        buttons |= 1 << 1;

    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), *m_camera.GetCamera());

    const state::PlayerState &currPlayer = m_worldState.m_players[m_worldState.m_currentPlayerId];

    Vector2 playerPos = {currPlayer.position.x, currPlayer.position.y};

    bool shootNow = buttons & (1 << 0);
    bool shootPrev = m_lastButtons & (1 << 0);

    if (shootNow && !shootPrev) {
        const Character::CharacterDef &charDef = Character::GetCharacterDef(m_currenCharacterId);

        Vector2 aimDir = Vector2Subtract(mouseWorld, playerPos);

        m_bulletSystem.Spawn({m_currentPlayerId, m_predictedPos, aimDir, charDef, m_localBulletSeq});

        m_localBulletSeq++;
    }

    m_lastButtons = buttons;
}

network::InputPacket GameScene::BuildInputPacket() {
    network::InputPacket packet{};

    if (!m_joined)
        return packet;

    packet.header.type = network::PacketType::Input;
    packet.playerId = m_worldState.m_currentPlayerId;
    packet.characterId = m_currenCharacterId;

    uint8_t buttons = 0;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        buttons |= 1 << 0;

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        buttons |= 1 << 1;

    packet.buttons = buttons;
    packet.sequence = m_inputSequence++;

    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), *m_camera.GetCamera());

    if (m_worldState.m_currentPlayerId != -1) {
        const state::PlayerState &currPlayer = m_worldState.m_players[m_worldState.m_currentPlayerId];

        Vector2 playerPos = {currPlayer.position.x, currPlayer.position.y};

        packet.facingAngle = atan2f(mouseWorld.y - playerPos.y, mouseWorld.x - playerPos.x);

        if (buttons & (1 << 0)) {
            Vector2 aimDir = Vector2Subtract(mouseWorld, playerPos);

            packet.aimX = aimDir.x;
            packet.aimY = aimDir.y;

            // match predicted bullet
            packet.predBulletSequence = m_localBulletSeq - 1;
        } else if (buttons & (1 << 1)) {
            packet.aimX = mouseWorld.x;
            packet.aimY = mouseWorld.y;
        }
    }

    return packet;
}

void GameScene::Reconcile(Vector2 serverPos, uint32_t ackedSeq) {
    state::PlayerState ghost = m_worldState.m_players[m_currentPlayerId];
    ghost.position = serverPos;

    std::vector<Collision::AABB> dynamicColliders = m_wallManager.GetColliders();
    for (size_t i = 0; i < INPUT_BUFFER_SIZE; ++i) {
        const PendingInput &pi = m_inputBuffer[i];
        if (pi.packet.sequence <= ackedSeq)
            continue;

        ghost.currentInput.moveX = pi.packet.moveX;
        ghost.currentInput.moveY = pi.packet.moveY;
        Character::SimulateMove(ghost, pi.dt, m_worldState.m_map.walls, dynamicColliders);
    }

    m_predictedPos = ghost.position;
}
