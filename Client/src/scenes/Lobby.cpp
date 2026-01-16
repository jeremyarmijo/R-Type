#include "scenes/Lobby.hpp"

extern "C" {
    Scene* CreateScene() {
        return new LobbyMenu();
    }
}
