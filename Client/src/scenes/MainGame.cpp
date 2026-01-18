#include "scenes/MainGame.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new MyGameScene();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new MyGameScene();
    }
}
#endif