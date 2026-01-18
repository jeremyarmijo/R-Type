#include "scenes/WaitLobby.hpp"

extern "C" {
    Scene* CreateScene() {
        return new WaitLobby();
    }
}
