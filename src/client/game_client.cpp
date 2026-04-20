#include "game_client.hpp"
#include "../network/packets/lobby_packets.hpp"
#include "scenes/lobby_scene.hpp"

void GameClient::Initialize() {
    int monitor = GetCurrentMonitor(); // or pick manually later

    int width = GetMonitorWidth(monitor);
    int height = GetMonitorHeight(monitor);

    InitWindow(width, height, "BruhBruh");

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

    m_sceneManager.Push(std::make_unique<LobbyScene>(m_events, m_transport, m_handler, m_sceneManager));
}

void GameClient::Start(const char *ip, int port) {
    Connect(ip, port);
    m_running = true;
    while (m_running) {
        Update();
    }
}

void GameClient::Update() {
    m_running = !WindowShouldClose();
    if (!m_running)
        return;

    float dt = GetFrameTime();

    network::InboundPacket pkt;
    while (m_transport.recv(pkt))
        m_handler.Dispatch(pkt.data, pkt.size);

    m_sceneManager.Update(dt);
    m_sceneManager.Render();
}

void GameClient::Disconnect() {
    network::DisconnectPacket packet{};
    packet.header.type = network::PacketType::Disconnect;
    // packet.playerId = m_gameScene.GetCurrentPlayerId(); // expose a getter
    m_transport.send(network::PEER_SERVER, &packet, sizeof(packet));
}

GameClient::~GameClient() {
    Disconnect();
    CloseWindow();
    // GameScene destructor handles its own Unload()
}

void GameClient::Connect(const char *ip, int port) { m_transport.connect(ip, port); }
