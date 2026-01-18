#include "scenes/JoinGame.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new JoinGame();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new JoinGame();
    }
}
#endif
