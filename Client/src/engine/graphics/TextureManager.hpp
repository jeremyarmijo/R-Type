#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>
#include <unordered_map>

class TextureManager {
 private:
  std::unordered_map<std::string, SDL_Texture*> m_textures;
  SDL_Renderer* m_renderer;

 public:
  TextureManager(SDL_Renderer* renderer) : m_renderer(renderer) {}

  ~TextureManager() {
    for (auto& pair : m_textures) {
      SDL_DestroyTexture(pair.second);
    }
  }

  bool LoadTexture(const std::string& key, const std::string& filepath);

  SDL_Texture* GetTexture(const std::string& key);

  void GetTextureSize(const std::string& key, int& width, int& height);
};
