#include "scenes/GameOver.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
    return new GameOverScene();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new GameOverScene();
    }
}
#endif