#include "scenes/LobbyInfo.hpp"

extern "C" {
    Scene* CreateScene() {
        return new LobbyInfoPlayer();
    }
}
