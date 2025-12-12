#include "ui/UIButton.hpp"
#include <iostream>

UIButton::~UIButton() {
  if (m_textTexture) {
    SDL_DestroyTexture(m_textTexture);
    m_textTexture = nullptr;
  }
  if (m_font) {
    TTF_CloseFont(m_font);
    m_font = nullptr;
  }
}

void UIButton::SetText(const std::string& text) {
  if (m_text != text) {
    m_text = text;
    m_needsTextUpdate = true;
  }
}

void UIButton::SetColors(SDL_Color normal, SDL_Color hover, SDL_Color pressed) {
  m_normalColor = normal;
  m_hoverColor = hover;
  m_pressedColor = pressed;
}

void UIButton::SetTextColor(SDL_Color normal, SDL_Color hover) {
  m_textColor = normal;
  m_textHoverColor = hover;
  m_needsTextUpdate = true;
}

void UIButton::SetTextColor(SDL_Color color) {
  m_textColor = color;
  m_textHoverColor = color;
  m_needsTextUpdate = true;
}

void UIButton::SetFontSize(int size) {
  if (m_fontSize != size) {
    m_fontSize = size;
    if (m_font) {
      TTF_CloseFont(m_font);
      m_font = nullptr;
    }
    m_needsTextUpdate = true;
  }
}

bool UIButton::LoadFont() {
  if (m_font) {
    return true;
  }

  if (m_fontPath.empty()) {
    const char* defaultFonts[] = {
      "../Client/assets/Font.ttf"
    };

    for (const char* path : defaultFonts) {
      m_font = TTF_OpenFont(path, m_fontSize);
      if (m_font) {
        m_fontPath = path;
        break;
      }
    }
  } else {
    m_font = TTF_OpenFont(m_fontPath.c_str(), m_fontSize);
  }

  if (!m_font) {
    std::cerr << "Failed to load font for button: " << TTF_GetError() << std::endl;
    return false;
  }

  return true;
}

void UIButton::UpdateTextTexture(SDL_Renderer* renderer) {
  if (!LoadFont()) {
    return;
  }

  if (m_textTexture) {
    SDL_DestroyTexture(m_textTexture);
    m_textTexture = nullptr;
  }

  if (m_text.empty()) {
    m_textWidth = 0;
    m_textHeight = 0;
    return;
  }

  SDL_Color textColor = m_isHovered ? m_textHoverColor : m_textColor;
  
  SDL_Surface* surface = TTF_RenderText_Blended(m_font, m_text.c_str(), textColor);
  if (!surface) {
    std::cerr << "Failed to render button text: " << TTF_GetError() << std::endl;
    return;
  }

  m_textTexture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!m_textTexture) {
    std::cerr << "Failed to create texture from button text: " << SDL_GetError() << std::endl;
    SDL_FreeSurface(surface);
    return;
  }

  m_textWidth = surface->w;
  m_textHeight = surface->h;

  SDL_FreeSurface(surface);
  m_needsTextUpdate = false;
}

bool UIButton::IsPointInside(int x, int y) const {
  return (x >= m_rect.x && x < m_rect.x + m_rect.w &&
          y >= m_rect.y && y < m_rect.y + m_rect.h);
}

void UIButton::Render(SDL_Renderer* renderer, TextureManager* textures) {
  if (!m_visible) return;

  if (m_needsTextUpdate) {
    UpdateTextTexture(renderer);
  }

  SDL_Color bgColor = m_normalColor;
  if (!m_enabled) {
    bgColor = m_disabledColor;
  } else if (m_isPressed) {
    bgColor = m_pressedColor;
  } else if (m_isHovered) {
    bgColor = m_hoverColor;
  }

  if (!m_textureKey.empty() && textures) {
    SDL_Texture* texture = textures->GetTexture(m_textureKey);
    if (texture) {
      if (!m_enabled) {
        SDL_SetTextureColorMod(texture, m_disabledColor.r, m_disabledColor.g, m_disabledColor.b);
      } else if (m_isPressed) {
        SDL_SetTextureColorMod(texture, m_pressedColor.r, m_pressedColor.g, m_pressedColor.b);
      } else if (m_isHovered) {
        SDL_SetTextureColorMod(texture, m_hoverColor.r, m_hoverColor.g, m_hoverColor.b);
      } else {
        SDL_SetTextureColorMod(texture, 255, 255, 255);
      }
      SDL_RenderCopy(renderer, texture, nullptr, &m_rect);
    } else {
      SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
      SDL_RenderFillRect(renderer, &m_rect);
    }
  } else {
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &m_rect);
  }

  SDL_Color borderColor = m_isHovered ? SDL_Color{255, 255, 255, 255} : SDL_Color{200, 200, 200, 255};
  if (!m_enabled) {
    borderColor = {100, 100, 100, 255};
  }
  SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
  SDL_RenderDrawRect(renderer, &m_rect);

  if (m_textTexture && !m_text.empty()) {
    int textX = m_rect.x + (m_rect.w - m_textWidth) / 2;
    int textY = m_rect.y + (m_rect.h - m_textHeight) / 2;

    if (m_isPressed) {
      textY += 2;
    }
    
    SDL_Rect textRect = {textX, textY, m_textWidth, m_textHeight};

    if (!m_enabled) {
      SDL_SetTextureAlphaMod(m_textTexture, 128);
    } else {
      SDL_SetTextureAlphaMod(m_textTexture, 255);
    }
    
    SDL_RenderCopy(renderer, m_textTexture, nullptr, &textRect);
  }
}

bool UIButton::HandleEvent(const SDL_Event& event) {
  if (!m_visible) return false;
  
  if (event.type == SDL_MOUSEMOTION) {
    int x = event.motion.x;
    int y = event.motion.y;
    
    bool wasHovered = m_isHovered;
    m_isHovered = IsPointInside(x, y);

    if (wasHovered != m_isHovered) {
      m_needsTextUpdate = true;
    }
    
    return false;
  }
  
  if (!m_enabled) return false;
  
  if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
    int x = event.button.x;
    int y = event.button.y;
    
    if (IsPointInside(x, y)) {
      m_isPressed = true;
      return true;
    }
  }
  
  if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
    if (m_isPressed) {
      m_isPressed = false;
      
      int x = event.button.x;
      int y = event.button.y;

      if (IsPointInside(x, y)) {
        if (m_onClick) {
          std::cout << "Button clicked: " << m_text << std::endl;
          m_onClick();
        }
        return true;
      }
    }
  }
  
  return false;
}
