#include "scenes/LobbyPassword.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new LobbyPassword();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new LobbyPassword();
    }
}
#endif