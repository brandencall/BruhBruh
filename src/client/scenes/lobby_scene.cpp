#include "lobby_scene.hpp"
#include "../scenes/game_scene.hpp"
#include "raylib.h"
#include <cstring>
#include <string>

LobbyScene::LobbyScene(Client::EventHub &events, network::ClientTransport &transport, NetworkMessageHandler &handler,
                       SceneManager &sceneManager)
    : m_events(events), m_transport(transport), m_handler(handler), m_sceneManager(sceneManager) {}

void LobbyScene::OnEnter() {
    using PT = network::PacketType;
    m_handler.Register(PT::JoinResponse, [this](const char *b) { HandleJoinResponse(b); });
    m_handler.Register(PT::PlayerJoined, [this](const char *b) { HandlePlayerJoined(b); });
    m_handler.Register(PT::LobbyState, [this](const char *b) { HandleLobbyState(b); });
    m_handler.Register(PT::StartGame, [this](const char *b) { HandleGameStarting(b); });
}

void LobbyScene::OnExit() {
    using PT = network::PacketType;
    m_handler.Unregister(PT::JoinResponse);
    m_handler.Unregister(PT::PlayerJoined);
    m_handler.Unregister(PT::LobbyState);
    m_handler.Unregister(PT::StartGame);

    Scene::OnExit();
}

void LobbyScene::Update(float dt) {
    if (!m_joined) {
        m_joinRetryAccumulator += dt;
        if (m_joinRetryAccumulator >= 1.0f) {
            SendJoin();
            m_joinRetryAccumulator = 0.0f;
        }
        return;
    }
    if (IsKeyPressed(KEY_SPACE) && !m_ready) {
        SendReady();
        m_ready = true;
    }
}

void LobbyScene::SendJoin() {
    network::JoinLobbyPacket pkt{};
    pkt.header.type = network::PacketType::JoinLobby;
    m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::HandleJoinResponse(const char *buf) {
    auto *pkt = reinterpret_cast<const network::JoinResponsePacket *>(buf);
    m_localPlayerId = pkt->playerId;
    m_players[m_localPlayerId].id = pkt->playerId;
    m_players[m_localPlayerId].characterId = pkt->characterId;
    m_joined = true;
    strcpy(m_players[m_localPlayerId].name, pkt->name);
}

void LobbyScene::HandlePlayerJoined(const char *buf) {
    auto *pkt = reinterpret_cast<const network::PlayerJoinedPacket *>(buf);
    m_players[pkt->playerId].id = pkt->playerId;
    m_players[pkt->playerId].characterId = pkt->characterId;
    strcpy(m_players[m_localPlayerId].name, pkt->name);
}

void LobbyScene::HandleLobbyState(const char *buf) {
    auto *pkt = reinterpret_cast<const network::LobbyStatePacket *>(buf);
    // m_localPlayerId = pkt->yourPlayerId;

    for (uint8_t i = 0; i < MAX_PLAYERS; ++i) {
        m_players[i].id = pkt->lobby[i].id;
        m_players[i].ready = pkt->lobby[i].ready;
        strcpy(m_players[i].name, pkt->lobby[i].name);
    }
}

void LobbyScene::HandleGameStarting(const char *buf) {
    // Server says everyone is ready — transition to game
    m_sceneManager.Replace(std::make_unique<GameScene>(m_events, m_transport, m_handler, m_sceneManager));
}

void LobbyScene::SendReady() {
    network::PlayerReadyPacket pkt{};
    pkt.header.type = network::PacketType::PlayerReady;
    pkt.playerId = m_localPlayerId;
    m_transport.send(network::PEER_SERVER, &pkt, sizeof(pkt));
}

void LobbyScene::Render() {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText("LOBBY", 560, 60, 32, WHITE);
    DrawText("Press SPACE when ready", 460, 660, 20, GRAY);

    for (int i = 0; i < 4; ++i) {
        int x = 160 + (i * 260);
        int y = 200;
        RenderPlayerSlot(i, m_players[i], x, y);
    }

    EndDrawing();
}

void LobbyScene::RenderPlayerSlot(int slot, const state::LobbySlotState &player, int x, int y) {
    Color borderColor = player.id != -1 ? WHITE : DARKGRAY;
    DrawRectangleLines(x, y, 220, 300, borderColor);

    if (player.id == -1) {
        DrawText("Waiting...", x + 60, y + 130, 16, DARKGRAY);
        return;
    }

    std::string name = player.name;
    DrawText(name.c_str(), x + 10, y + 20, 18, WHITE);

    if (player.ready) {
        DrawText("READY", x + 75, y + 240, 20, GREEN);
    } else {
        DrawText("NOT READY", x + 55, y + 240, 20, RED);
    }

    // Highlight local player slot
    if (player.id == m_localPlayerId) {
        DrawRectangleLines(x - 2, y - 2, 224, 304, YELLOW);
    }
}
