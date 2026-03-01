#include "client.hpp"
#include "game.hpp"
#include "network/server.hpp"
#include <iostream>

void RunServer() {
    std::cout << "Running the server" << std::endl;
    network::Server server;
    server.Start(54000);

    while (server.IsRunning()) {
        // server.Tick();
        server.Receive();
    }
}

void RunClient() {
    std::cout << "Running the client" << std::endl;
    network::Client client;
    client.Connect("127.0.0.1", 54000);
    client.SendJoin();

    while (client.IsRunning()) {
        // client.Send("Hello", 5);
        client.Update();
    }
}

int main(int argc, char **argv) {
    bool isServer = false;
    bool isClient = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--server")
            isServer = true;
        else if (arg == "--client")
            isClient = true;
    }

    if (isServer)
        RunServer();
    else if (isClient)
        RunClient();
    else
        RunClient(); // default
    // Game game;
    // while (game.GameRunning()) {
    //     game.Update();
    // }
}
