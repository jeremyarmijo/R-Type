#pragma once
#include <SDL2/SDL.h>

#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "graphics/AnimationManager.hpp"
#include "graphics/RenderComponents.hpp"
#include "graphics/TextureManager.hpp"
#include "physics/Physics2D.hpp"

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
