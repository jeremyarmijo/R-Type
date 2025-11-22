#pragma once
#include <SDL2/SDL.h>

#include "engine/graphics/AnimationManager.hpp"
#include "engine/graphics/TextureManager.hpp"
#include "engine/graphics/RenderComponents.hpp"
#include "engine/physics/Physics2D.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Zipper.hpp"

void animation_system(Registry& registry, SparseArray<Animation>& animations,
                      SparseArray<Sprite>& sprites,
                      AnimationManager* animationManager, float deltaTime);

void sprite_render_system(Registry& registry,
                          const SparseArray<Transform>& transforms,
                          const SparseArray<Sprite>& sprites,
                          TextureManager* textureManager,
                          SDL_Renderer* renderer,
                          const Vector2& cameraPosition = {0, 0});

void layered_sprite_render_system(Registry& registry,
                                  const SparseArray<Transform>& transforms,
                                  const SparseArray<Sprite>& sprites,
                                  TextureManager* textureManager,
                                  SDL_Renderer* renderer,
                                  const Vector2& cameraPosition = {0, 0});

Vector2 get_camera_position(Registry& registry,
                            const SparseArray<Camera>& cameras,
                            const SparseArray<Transform>& transforms);
