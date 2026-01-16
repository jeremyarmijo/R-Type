#include "scenes/LobbyPassword.hpp"

extern "C" {
    Scene* CreateScene() {
        return new LobbyPassword();
    }
}
