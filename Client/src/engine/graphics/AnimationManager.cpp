#include <vector>
#include <string>

#include "engine/graphics/AnimationManager.hpp"

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
  return (it != m_animations.end()) ? &it->second : nullptr;
}
