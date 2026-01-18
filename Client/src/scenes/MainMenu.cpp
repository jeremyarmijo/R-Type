#include "scenes/MainMenu.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new MainMenu();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new MainMenu();
    }
}
#endif
