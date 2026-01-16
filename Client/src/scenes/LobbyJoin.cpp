#include "scenes/LobbyJoin.hpp"

extern "C" {
    Scene* CreateScene() {
        return new LobbyJoin();
    }
}
