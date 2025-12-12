#include "ui/UISolidColor.hpp"

void UISolidColor::Render(SDL_Renderer* renderer, TextureManager* textures) {
  if (!m_visible) return;

  SDL_Rect rect = m_rect;

  SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
  SDL_RenderFillRect(renderer, &rect);
}
