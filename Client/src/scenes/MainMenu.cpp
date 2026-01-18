#include "scenes/MainMenu.hpp"

extern "C" {
    Scene* CreateScene() {
        return new MainMenu();
    }
}
