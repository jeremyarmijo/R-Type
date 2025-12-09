#pragma once
#include <SDL2/SDL.h>

#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "graphics/AnimationManager.hpp"
#include "graphics/RenderComponents.hpp"
#include "graphics/TextureManager.hpp"

void animation_system(Registry& registry, SparseArray<Animation>& animations,
                      SparseArray<Sprite>& sprites,
                      AnimationManager* animationManager, float deltaTime);

Vector2 get_camera_position(Registry& registry,
                            const SparseArray<Camera>& cameras,
                            const SparseArray<Transform>& transforms);
