#include "graphics/RenderSystem.hpp"

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
