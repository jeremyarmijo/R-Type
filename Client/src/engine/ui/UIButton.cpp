#include "ui/UIButton.hpp"

#include <iostream>

bool UIButton::IsPointInside(int x, int y) const {
  return (x >= m_rect.x && x < m_rect.x + m_rect.w && y >= m_rect.y &&
          y < m_rect.y + m_rect.h);
}

void UIButton::Render(SDL_Renderer* renderer, TextureManager* textures) {
  if (!m_visible) return;

  // Determine color based on state
  SDL_Color color = m_normalColor;
  if (m_isPressed) {
    color = m_pressedColor;
  } else if (m_isHovered) {
    color = m_hoverColor;
  }

  // Render texture or colored rectangle
  if (!m_textureKey.empty() && textures) {
    SDL_Texture* texture = textures->GetTexture(m_textureKey);
    if (texture) {
      SDL_RenderCopy(renderer, texture, nullptr, &m_rect);
      return;
    }
  }

  // Fallback: render as colored rectangle
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(renderer, &m_rect);

  // Draw border
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderDrawRect(renderer, &m_rect);

  // Note: Text rendering would require UIText integration or SDL_ttf here
  // For now, this is a simple button without text rendering
  // To add text, you could embed a UIText element or use SDL_ttf directly
}

bool UIButton::HandleEvent(const SDL_Event& event) {
  if (!m_visible || !m_enabled) return false;

  if (event.type == SDL_MOUSEMOTION) {
    int x = event.motion.x;
    int y = event.motion.y;

    m_isHovered = IsPointInside(x, y);
    return false;  // Don't consume motion events
  }

  if (event.type == SDL_MOUSEBUTTONDOWN &&
      event.button.button == SDL_BUTTON_LEFT) {
    int x = event.button.x;
    int y = event.button.y;

    if (IsPointInside(x, y)) {
      m_isPressed = true;
      return true;  // Consume the event
    }
  }

  if (event.type == SDL_MOUSEBUTTONUP &&
      event.button.button == SDL_BUTTON_LEFT) {
    if (m_isPressed) {
      m_isPressed = false;

      int x = event.button.x;
      int y = event.button.y;

      // Only trigger onClick if released over the button
      if (IsPointInside(x, y)) {
        if (m_onClick) {
          std::cout << "Button clicked: " << m_text << std::endl;
          m_onClick();
        }
        return true;  // Consume the event
      }
    }
  }

  return false;
}
