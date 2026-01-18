#include "scenes/LobbyJoin.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new LobbyJoin();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new LobbyJoin();
    }
}
#endif