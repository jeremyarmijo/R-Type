#pragma once
#include <SDL2/SDL.h>
#include <string>
#include "ui/UIElement.hpp"

class UIImage : public UIElement {
private:
  std::string m_textureKey;
  SDL_Rect m_sourceRect;
  bool m_maintainAspectRatio;

public:
  UIImage(int x, int y, int w, int h, const std::string& textureKey = "")
    : UIElement(x, y, w, h),
      m_textureKey(textureKey),
      m_sourceRect{0, 0, 0, 0},
      m_maintainAspectRatio(false) {}

  void SetTexture(const std::string& key) { m_textureKey = key; }
  void SetSourceRect(SDL_Rect rect) { m_sourceRect = rect; }
  void SetMaintainAspectRatio(bool maintain) { m_maintainAspectRatio = maintain; }
  
  const std::string& GetTexture() const { return m_textureKey; }

  void Render(SDL_Renderer* renderer, TextureManager* textures) override;
};
