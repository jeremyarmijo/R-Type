#include "graphics/AnimationManager.hpp"

#include <iostream>
#include <string>
#include <vector>

void AnimationManager::CreateAnimation(
    const std::string& name, const std::string& textureKey,
    const std::vector<AnimationFrame>& frames, bool loop) {
  AnimationClip clip(loop);
  clip.textureKey = textureKey;
  clip.frames = frames;
  m_animations[name] = clip;
}

const AnimationClip* AnimationManager::GetAnimation(
    const std::string& name) const {
  auto it = m_animations.find(name);
  if (it == m_animations.end())
    std::cout << "failed to find texture: " << name << std::endl;
  return (it != m_animations.end()) ? &it->second : nullptr;
}
