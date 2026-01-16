#include "scenes/Options.hpp"

extern "C" {
    Scene* CreateScene() {
        return new OptionsScene();
    }
}
