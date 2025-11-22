#include "engine/graphics/RenderSystem.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

void animation_system(Registry& registry, SparseArray<Animation>& animations,
                      SparseArray<Sprite>& sprites,
                      AnimationManager* animationManager, float deltaTime) {
  for (auto&& [animation, sprite] : Zipper(animations, sprites)) {
    if (!animation.isPlaying || animation.currentAnimation.empty()) {
      continue;
    }

    const AnimationClip* clip =
        animationManager->GetAnimation(animation.currentAnimation);
    if (!clip || clip->frames.empty()) {
      continue;
    }

    animation.currentTime += deltaTime;

    const AnimationFrame& currentFrame = clip->frames[animation.currentFrame];

    if (animation.currentTime >= currentFrame.duration) {
      animation.currentTime = 0;
      animation.currentFrame++;

      if (animation.currentFrame >= static_cast<int>(clip->frames.size())) {
        if (clip->loop) {
          animation.currentFrame = 0;
        } else {
          animation.currentFrame = static_cast<int>(clip->frames.size()) - 1;
          animation.isPlaying = false;
        }
      }
    }

    if (animation.currentFrame < static_cast<int>(clip->frames.size())) {
      sprite.sourceRect = clip->frames[animation.currentFrame].sourceRect;
    }
  }
}

void sprite_render_system(Registry& registry,
                          const SparseArray<Transform>& transforms,
                          const SparseArray<Sprite>& sprites,
                          TextureManager* textureManager,
                          SDL_Renderer* renderer,
                          const Vector2& cameraPosition) {
  for (auto&& [transform, sprite] : Zipper(transforms, sprites)) {
    SDL_Texture* texture = textureManager->GetTexture(sprite.textureKey);
    if (!texture) {
      continue;
    }

    SDL_Rect sourceRect = sprite.sourceRect;
    if (sourceRect.w == 0 || sourceRect.h == 0) {
      int textureWidth, textureHeight;
      textureManager->GetTextureSize(sprite.textureKey, textureWidth,
                                     textureHeight);
      sourceRect = {0, 0, textureWidth, textureHeight};
    }

    SDL_Rect destRect = {
        static_cast<int>(transform.position.x - cameraPosition.x -
                         (sourceRect.w * transform.scale.x * sprite.pivot.x)),
        static_cast<int>(transform.position.y - cameraPosition.y -
                         (sourceRect.h * transform.scale.y * sprite.pivot.y)),
        static_cast<int>(sourceRect.w * transform.scale.x),
        static_cast<int>(sourceRect.h * transform.scale.y)};

    SDL_RenderCopyEx(renderer, texture, &sourceRect, &destRect,
                     transform.rotation, nullptr, SDL_FLIP_NONE);
  }
}

void layered_sprite_render_system(Registry& registry,
                                  const SparseArray<Transform>& transforms,
                                  const SparseArray<Sprite>& sprites,
                                  TextureManager* textureManager,
                                  SDL_Renderer* renderer,
                                  const Vector2& cameraPosition) {
  struct RenderData {
    size_t entityIndex;
    const Transform* transform;
    const Sprite* sprite;
  };

  std::vector<RenderData> renderList;

  for (auto&& [idx, transform, sprite] : IndexedZipper(transforms, sprites)) {
    renderList.push_back({idx, &transform, &sprite});
  }

  std::sort(renderList.begin(), renderList.end(),
            [](const RenderData& a, const RenderData& b) {
              return a.sprite->layer < b.sprite->layer;
            });

  for (const auto& data : renderList) {
    SDL_Texture* texture = textureManager->GetTexture(data.sprite->textureKey);
    if (!texture) continue;

    SDL_Rect sourceRect = data.sprite->sourceRect;
    if (sourceRect.w == 0 || sourceRect.h == 0) {
      int textureWidth, textureHeight;
      textureManager->GetTextureSize(data.sprite->textureKey, textureWidth,
                                     textureHeight);
      sourceRect = {0, 0, textureWidth, textureHeight};
    }

    SDL_Rect destRect = {
        static_cast<int>(
            data.transform->position.x - cameraPosition.x -
            (sourceRect.w * data.transform->scale.x * data.sprite->pivot.x)),
        static_cast<int>(
            data.transform->position.y - cameraPosition.y -
            (sourceRect.h * data.transform->scale.y * data.sprite->pivot.y)),
        static_cast<int>(sourceRect.w * data.transform->scale.x),
        static_cast<int>(sourceRect.h * data.transform->scale.y)};

    SDL_RenderCopyEx(renderer, texture, &sourceRect, &destRect,
                     data.transform->rotation, nullptr, SDL_FLIP_NONE);
  }
}

Vector2 get_camera_position(Registry& registry,
                            const SparseArray<Camera>& cameras,
                            const SparseArray<Transform>& transforms) {
  for (auto&& [idx, camera, transform] : IndexedZipper(cameras, transforms)) {
    if (camera.isActive) {
      return camera.position;
    }
  }

  return {0, 0};
}
