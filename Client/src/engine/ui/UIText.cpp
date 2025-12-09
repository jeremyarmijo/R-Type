#include "ui/UIText.hpp"

#include <iostream>

UIText::UIText(int x, int y, const std::string& text,
               const std::string& fontPath, int fontSize, SDL_Color color,
               UIAnchor anchor)
    : UIElement(x, y, 0, 0, anchor),
      m_text(text),
      m_fontPath(fontPath),
      m_fontSize(fontSize),
      m_color(color),
      m_alignment(TextAlign::Left),
      m_font(nullptr),
      m_textTexture(nullptr),
      m_needsUpdate(true),
      m_textWidth(0),
      m_textHeight(0) {}

UIText::~UIText() {
  if (m_textTexture) {
    SDL_DestroyTexture(m_textTexture);
    m_textTexture = nullptr;
  }
  if (m_font) {
    TTF_CloseFont(m_font);
    m_font = nullptr;
  }
}

void UIText::SetText(const std::string& text) {
  if (m_text != text) {
    m_text = text;
    m_needsUpdate = true;
  }
}

void UIText::SetColor(SDL_Color color) {
  if (m_color.r != color.r || m_color.g != color.g || m_color.b != color.b ||
      m_color.a != color.a) {
    m_color = color;
    m_needsUpdate = true;
  }
}

void UIText::SetFontSize(int size) {
  if (m_fontSize != size) {
    m_fontSize = size;
    if (m_font) {
      TTF_CloseFont(m_font);
      m_font = nullptr;
    }
    m_needsUpdate = true;
  }
}

void UIText::SetAlignment(TextAlign align) { m_alignment = align; }

bool UIText::LoadFont() {
  if (m_font) {
    return true;  // Already loaded
  }

  if (m_fontPath.empty()) {
    // Try default font paths
    const char* defaultFonts[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",  // Linux
        "/System/Library/Fonts/Helvetica.ttc",              // macOS
        "C:\\Windows\\Fonts\\arial.ttf",                    // Windows
        "../assets/fonts/default.ttf",                      // Project relative
        "./assets/fonts/default.ttf",
        "assets/fonts/default.ttf"};

    for (const char* path : defaultFonts) {
      m_font = TTF_OpenFont(path, m_fontSize);
      if (m_font) {
        m_fontPath = path;
        std::cout << "Loaded font: " << path << std::endl;
        break;
      }
    }
  } else {
    m_font = TTF_OpenFont(m_fontPath.c_str(), m_fontSize);
  }

  if (!m_font) {
    std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    return false;
  }

  return true;
}

void UIText::UpdateTextTexture(SDL_Renderer* renderer) {
  if (!LoadFont()) {
    return;
  }

  // Destroy old texture
  if (m_textTexture) {
    SDL_DestroyTexture(m_textTexture);
    m_textTexture = nullptr;
  }

  if (m_text.empty()) {
    m_textWidth = 0;
    m_textHeight = 0;
    return;
  }

  // Render text to surface using blended mode for anti-aliasing
  SDL_Surface* surface =
      TTF_RenderText_Blended(m_font, m_text.c_str(), m_color);
  if (!surface) {
    std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
    return;
  }

  // Create texture from surface
  m_textTexture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!m_textTexture) {
    std::cerr << "Failed to create texture from text: " << SDL_GetError()
              << std::endl;
    SDL_FreeSurface(surface);
    return;
  }

  // Get dimensions
  m_textWidth = surface->w;
  m_textHeight = surface->h;

  SDL_FreeSurface(surface);
  m_needsUpdate = false;
}

void UIText::Update(float deltaTime) {
  // Can be used for text animations, blinking, etc.
  // Currently does nothing, but available for override
}

void UIText::Render(SDL_Renderer* renderer, TextureManager* textures) {
  if (!m_visible) return;

  if (m_needsUpdate) {
    UpdateTextTexture(renderer);
  }

  if (!m_textTexture || m_text.empty()) {
    return;
  }

  // Calculate position based on alignment
  int renderX = m_rect.x;
  int renderY = m_rect.y;

  switch (m_alignment) {
    case TextAlign::Center:
      renderX -= m_textWidth / 2;
      break;
    case TextAlign::Right:
      renderX -= m_textWidth;
      break;
    case TextAlign::Left:
    default:
      break;
  }

  SDL_Rect destRect = {renderX, renderY, m_textWidth, m_textHeight};
  SDL_RenderCopy(renderer, m_textTexture, nullptr, &destRect);
}
