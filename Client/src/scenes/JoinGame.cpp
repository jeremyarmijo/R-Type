#include "scenes/JoinGame.hpp"

extern "C" {
    Scene* CreateScene() {
        return new JoinGame();
    }
}
