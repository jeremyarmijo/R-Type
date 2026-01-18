#include "scenes/LevelScene.hpp"

extern "C" {
Scene* CreateScene() { return new Level("level"); }
}
