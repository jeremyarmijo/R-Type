#include "scenes/CreateLobby.hpp"


#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new CreateLobby();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new CreateLobby();
    }
}
#endif