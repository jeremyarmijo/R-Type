#include "scenes/Options.hpp"

#ifdef _WIN32
    __declspec(dllexport) Scene* CreateScene() {
        return new OptionsScene();
    }
#else
extern "C" {
    Scene* CreateScene() {
        return new OptionsScene();
    }
}
#endif