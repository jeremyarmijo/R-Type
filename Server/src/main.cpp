#include <iostream>
#include "network/ServerNetworkManager.hpp"
// #include "game/GameLogic.hpp"

int main() {
    // Initialisation
    ServerNetworkManager network_manager;
    // GameLogic game_logic;
    
    // Initialize avec TCP (4242) et UDP (4243)
    if (!network_manager.Initialize(4242, 4243)) {
        std::cerr << "Failed to initialize network manager" << std::endl;
        return 1;
    }
    
    std::cout << "R-Type Server started!" << std::endl;
    std::cout << "   TCP (Login): port 4242" << std::endl;
    std::cout << "   UDP (Gameplay): port 4243" << std::endl;
    std::cout << std::endl;
    
    // Configuration des callbacks
    // network_manager.SetMessageCallback([&game_logic](const NetworkMessage& msg) {
    //     game_logic.ProcessNetworkMessage(msg);
    // });

    network_manager.SetMessageCallback([](const NetworkMessage& msg) {
        std::cout << "📩 UDP Message from client " << msg.client_id 
                  << " (" << msg.data.size() << " bytes): ";
        for (auto byte : msg.data) {
            printf("%02X ", byte);
        }
        std::cout << std::endl;
    });

    network_manager.SetConnectionCallback([](uint32_t client_id) {
        std::cout << "🔌 Client " << client_id << " connected!" << std::endl;
        // game_logic.OnPlayerConnected(client_id);
    });

    network_manager.SetDisconnectionCallback([](uint32_t client_id) {
        std::cout << "🔌 Client " << client_id << " disconnected!" << std::endl;
        // game_logic.OnPlayerDisconnected(client_id);
    });
    
    // Game loop
    const auto frame_duration = std::chrono::milliseconds(16); // 60 FPS
    auto last_time = std::chrono::steady_clock::now();
    
    while (true) {
        auto current_time = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration<float>(current_time - last_time).count();

        last_time = current_time;
        
        // 1. Traiter les messages réseau entrants
        network_manager.Update();
        
        // 2. Update logique de jeu
        // game_logic.Update(delta);
        
        // 3. Envoyer les updates aux clients
        // auto game_state = game_logic.GetGameState();
        NetworkMessage msg;
        msg.data = {0xDE, 0xAD, 0xBE, 0xEF}; // Exemple de données
        // msg.data = SerializeGameState(game_state);
        network_manager.Broadcast(msg);
        
        // 4. Frame rate limiting
        auto frame_time = std::chrono::steady_clock::now() - current_time;
        if (frame_time < frame_duration) {
            std::this_thread::sleep_for(frame_duration - frame_time);
        }
    }
    
    network_manager.Shutdown();
    return 0;
}
