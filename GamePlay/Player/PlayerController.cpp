#include "PlayerController.hpp"
#include <SDL2/SDL.h>
#include "components/Physics2D.hpp"
#include "ecs/Zipper.hpp"

Vector2 PlayerController::get_input_direction() const {
  Vector2 dir = {0.f, 0.f};
  const Uint8* keys = SDL_GetKeyboardState(nullptr);

  if (keys[SDL_SCANCODE_W])  // UP
    dir.y = -1.f;
  if (keys[SDL_SCANCODE_S])  // Down
    dir.y = 1.f;
  if (keys[SDL_SCANCODE_A])  // Gauche
    dir.x = -1.f;
  if (keys[SDL_SCANCODE_D])  // Droite
    dir.x = 1.f;
  return dir;
}
