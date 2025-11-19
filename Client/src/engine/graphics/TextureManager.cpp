#include "TextureManager.hpp"

#include <iostream>

bool TextureManager::LoadTexture(const std::string& key,
                                 const std::string& filepath) {
  SDL_Surface* surface = IMG_Load(filepath.c_str());
  if (!surface) {
    std::cout << "failed to load " << filepath << " error: " << IMG_GetError()
              << std::endl;
    return false;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
  SDL_FreeSurface(surface);

  if (!texture) return false;

  m_textures[key] = texture;
  std::cout << key << " texture key loaded" << std::endl;
  return true;
}

SDL_Texture* TextureManager::GetTexture(const std::string& key) {
  auto it = m_textures.find(key);
  return (it != m_textures.end()) ? it->second : nullptr;
}

void TextureManager::GetTextureSize(const std::string& key, int& width,
                                    int& height) {
  SDL_Texture* texture = GetTexture(key);
  if (texture) {
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
  }
}
