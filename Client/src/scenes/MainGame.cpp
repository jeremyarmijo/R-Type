#include "scenes/MainGame.hpp"

extern "C" {
    Scene* CreateScene() {
        return new MyGameScene();
    }
}
