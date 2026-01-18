#include "scenes/WaitLobby.hpp"
#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new WaitLobby();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new WaitLobby();
    }
}
#endif