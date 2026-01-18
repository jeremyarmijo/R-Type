#include "scenes/CreateLobby.hpp"

extern "C" {
    Scene* CreateScene() {
        return new CreateLobby();
    }
}
