#include "scenes/LobbyInfo.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new LobbyInfoPlayer();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new LobbyInfoPlayer();
    }
}
#endif