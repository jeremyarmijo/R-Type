#include <iostream>
#include "include/ServerMacro.hpp"
#include "network/ServerNetworkManager.hpp"
// #include "game/GameLogic.hpp"

int main() {
    ServerNetworkManager network_manager;
    // GameLogic game_logic;
    
    if (!network_manager.Initialize(PORT_TCP_DEFAULT, PORT_UDP_DEFAULT)) {
        std::cerr << "Failed to initialize network manager" << std::endl;
        return 1;
    }
    
    std::cout << std::endl << "R-Type Server started!" << std::endl;
    std::cout << "TCP: port " << PORT_TCP_DEFAULT << std::endl;
    std::cout << "UDP: port " << PORT_UDP_DEFAULT << std::endl;

    // Configuration des callbacks
    // network_manager.SetMessageCallback([&game_logic](const NetworkMessage& msg) {
    //     game_logic.ProcessNetworkMessage(msg);
    // });

    network_manager.SetMessageCallback([](const NetworkMessage& msg) {
        std::cout << "UDP Message from client " << msg.client_id 
                  << " (" << msg.data.size() << " bytes): ";
        for (auto byte : msg.data) {
            printf("%02X ", byte);
        }
        std::cout << std::endl;
    });

    network_manager.SetConnectionCallback([](uint32_t client_id) {
        std::cout << "Client " << client_id << " connected!" << std::endl;
        // game_logic.OnPlayerConnected(client_id);
    });

    network_manager.SetDisconnectionCallback([](uint32_t client_id) {
        std::cout << "Client " << client_id << " disconnected!" << std::endl;
        // game_logic.OnPlayerDisconnected(client_id);
    });

    const auto frame_duration = std::chrono::milliseconds(16); // 60 FPS
    auto last_time = std::chrono::steady_clock::now();
    
    while (true) {
        auto current_time = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration<float>(current_time - last_time).count();

        last_time = current_time;
        network_manager.Update();
        // game_logic.Update(delta);

        // auto game_state = game_logic.GetGameState();
        NetworkMessage msg;
        msg.data = {0xDE, 0xAD, 0xBE, 0xEF};
        // msg.data = SerializeGameState(game_state);
        network_manager.Broadcast(msg);

        auto frame_time = std::chrono::steady_clock::now() - current_time;
        if (frame_time < frame_duration) {
            std::this_thread::sleep_for(frame_duration - frame_time);
        }
    }
    
    network_manager.Shutdown();
    return 0;
}
