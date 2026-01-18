#include "scenes/Lobby.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new LobbyMenu();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new LobbyMenu();
    }
}
#endif