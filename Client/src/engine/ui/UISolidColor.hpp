#pragma once
#include <SDL2/SDL.h>

#include "ui/UIElement.hpp"

class UISolidColor : public UIElement {
 private:
  SDL_Color m_color;

 public:
  UISolidColor(int x, int y, int w, int h, SDL_Color color,
               UIAnchor anchor = UIAnchor::TopLeft)
      : UIElement(x, y, w, h, anchor), m_color(color) {}

  void SetColor(SDL_Color c) { m_color = c; }
  SDL_Color GetColor() const { return m_color; }

  void Render(SDL_Renderer* renderer, TextureManager* textures) override;
};
