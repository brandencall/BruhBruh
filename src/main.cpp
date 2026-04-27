#include "session_manager.hpp"
#include "steam/steam_api.h"
#include <iostream>

int main() {
    if (!SteamAPI_Init()) {
        std::cerr << "SteamAPI_Init failed. Is Steam running?\n";
        return 1;
    }

    // network::SteamTransport transport;
    // transport.Init();

    // SteamLobbyManager lobbyManager(transport);

    SessionManager sessionManager;

    bool isHost = false;

    std::cout << "Host (h) or Join (j)? ";
    char choice;
    std::cin >> choice;
    isHost = (choice == 'h' || choice == 'H');
    sessionManager.Initialize();

    if (isHost) {
        sessionManager.StartHost();
    } else {
        sessionManager.StartClient();
    }

    sessionManager.Run(); // main loop (blocks)
    sessionManager.Shutdown();

    SteamAPI_Shutdown();
    return 0;
}

// int main() {
//     if (!SteamAPI_Init()) {
//         std::cerr << "SteamAPI_Init failed. Is Steam running?\n";
//         return 1;
//     }
//
//     // SteamTransport is shared between server and client (in-process)
//     network::SteamTransport transport;
//     transport.Init();
//
//     SteamLobbyManager lobbyManager(transport);
//
//     bool isHost = false;
//
//     // Show a simple pre-window menu to decide host vs join.
//     // In practice this becomes part of your lobby scene UI.
//     // For now a console prompt is fine for testing:
//     std::cout << "Host (h) or Join (j)? ";
//     char choice;
//     std::cin >> choice;
//     isHost = (choice == 'h' || choice == 'H');
//
//     if (isHost) {
//         // Start server on background thread
//         GameServer server;
//         std::thread serverThread([&]() {
//             server.StartInProcess(transport);
//             server.RunServer();
//         });
//
//         // Create Steam lobby so friends can join
//         lobbyManager.SetCallbacks(
//             {.onLobbyCreated = [&]() { std::cout << "Lobby created"; },
//              .onMemberJoined =
//                  [&](CSteamID who) { std::cout << SteamFriends()->GetFriendPersonaName(who) << " joined\n"; },
//              .onError = [](const char *msg) { std::cerr << "Lobby error: " << msg << "\n"; }});
//         lobbyManager.CreateLobby(MAX_PLAYERS);
//
//         GameClient client{transport, lobbyManager};
//         client.Initialize();
//         client.StartInProcess();
//
//         server.Stop();
//         serverThread.join();
//     } else {
//         // Client — lobby join happens via Steam overlay invite
//         // GameLobbyJoinRequested_t callback in SteamLobbyManager handles it
//         lobbyManager.SetCallbacks({.onLobbyJoined = [&]() { std::cout << "Joined lobby\n"; },
//                                    .onError = [](const char *msg) { std::cerr << "Lobby error: " << msg << "\n"; }});
//
//         GameClient client{transport, lobbyManager};
//         client.Initialize();
//         client.StartInProcess();
//     }
//
//     SteamAPI_Shutdown();
//     return 0;
// }

// #include "client/game_client.hpp"
// #include "game_server.hpp"
// #include <iostream>
//
// void RunServer() {
//     std::cout << "Running the server" << std::endl;
//     GameServer gameServer;
//     gameServer.Start(54000);
//     gameServer.RunServer();
// }
//
// void RunClient() {
//     GameClient gameClient;
//     gameClient.Initialize();
//     gameClient.Start("127.0.0.1", 54000);
// }
//
// int main(int argc, char **argv) {
//     bool isServer = false;
//     bool isClient = false;
//
//     for (int i = 1; i < argc; ++i) {
//         std::string arg = argv[i];
//
//         if (arg == "--server")
//             isServer = true;
//         else if (arg == "--client")
//             isClient = true;
//     }
//
//     if (isServer)
//         RunServer();
//     else if (isClient)
//         RunClient();
//     else
//         RunClient(); // default
// }
