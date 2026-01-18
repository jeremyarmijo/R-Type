#include "scenes/Victory.hpp"

extern "C" {
    Scene* CreateScene() {
        return new VictoryScene();
    }
}
