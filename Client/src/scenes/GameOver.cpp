#include "scenes/GameOver.hpp"

extern "C" {
    Scene* CreateScene() {
        return new GameOverScene();
    }
}
