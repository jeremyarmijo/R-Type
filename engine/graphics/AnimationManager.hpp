#pragma once
#include <SDL2/SDL.h>

#include <string>
#include <unordered_map>
#include <vector>

struct AnimationFrame {
  SDL_Rect sourceRect;
  float duration;
};

struct AnimationClip {
  std::string textureKey;
  std::vector<AnimationFrame> frames;
  bool loop;

  explicit AnimationClip(bool shouldLoop = true) : loop(shouldLoop) {}
};

class AnimationManager {
 private:
  std::unordered_map<std::string, AnimationClip> m_animations;

 public:
  void CreateAnimation(const std::string& name, const std::string& textureKey,
                       const std::vector<AnimationFrame>& frames,
                       bool loop = true);

  const AnimationClip* GetAnimation(const std::string& name) const;
};
