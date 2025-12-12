#include "ui/UITextInput.hpp"

#include <algorithm>
#include <iostream>
#include <string>

UITextInput::UITextInput(int x, int y, int w, int h,
                         const std::string& placeholder,
                         const std::string& fontPath, int fontSize,
                         UIAnchor anchor)
    : UIElement(x, y, w, h, anchor),
      m_text(""),
      m_placeholder(placeholder),
      m_fontPath(fontPath),
      m_fontSize(fontSize),
      m_textColor{255, 255, 255, 255},
      m_placeholderColor{128, 128, 128, 255},
      m_backgroundColor{40, 40, 50, 255},
      m_borderColor{100, 100, 120, 255},
      m_focusedBorderColor{100, 150, 255, 255},
      m_borderThickness(2),
      m_padding(8),
      m_maxLength(100),
      m_isFocused(false),
      m_isPassword(false),
      m_cursorPosition(0),
      m_cursorBlinkTime(0.0f),
      m_cursorVisible(true),
      m_font(nullptr),
      m_textTexture(nullptr),
      m_needsUpdate(true) {}

UITextInput::~UITextInput() {
  if (m_textTexture) {
    SDL_DestroyTexture(m_textTexture);
    m_textTexture = nullptr;
  }
  if (m_font) {
    TTF_CloseFont(m_font);
    m_font = nullptr;
  }
  // Stop text input if this was focused
  if (m_isFocused) {
    SDL_StopTextInput();
  }
}

void UITextInput::SetText(const std::string& text) {
  if (text.length() <= m_maxLength) {
    m_text = text;
    m_cursorPosition = m_text.length();
    m_needsUpdate = true;
    if (m_onTextChanged) {
      m_onTextChanged(m_text);
    }
  }
}

void UITextInput::SetPlaceholder(const std::string& placeholder) {
  m_placeholder = placeholder;
  if (m_text.empty()) {
    m_needsUpdate = true;
  }
}

void UITextInput::SetMaxLength(size_t length) {
  m_maxLength = length;
  if (m_text.length() > m_maxLength) {
    m_text = m_text.substr(0, m_maxLength);
    m_cursorPosition = std::min(m_cursorPosition, m_text.length());
    m_needsUpdate = true;
  }
}

void UITextInput::SetPassword(bool isPassword) {
  if (m_isPassword != isPassword) {
    m_isPassword = isPassword;
    m_needsUpdate = true;
  }
}

void UITextInput::SetTextColor(SDL_Color color) {
  m_textColor = color;
  m_needsUpdate = true;
}

void UITextInput::SetPlaceholderColor(SDL_Color color) {
  m_placeholderColor = color;
  if (m_text.empty()) {
    m_needsUpdate = true;
  }
}

void UITextInput::SetBackgroundColor(SDL_Color color) {
  m_backgroundColor = color;
}

void UITextInput::SetBorderColor(SDL_Color normal, SDL_Color focused) {
  m_borderColor = normal;
  m_focusedBorderColor = focused;
}

void UITextInput::SetBorderThickness(int thickness) {
  m_borderThickness = thickness;
}

void UITextInput::SetPadding(int padding) { m_padding = padding; }

void UITextInput::SetFontSize(int size) {
  if (m_fontSize != size) {
    m_fontSize = size;
    if (m_font) {
      TTF_CloseFont(m_font);
      m_font = nullptr;
    }
    m_needsUpdate = true;
  }
}

void UITextInput::SetOnTextChanged(
    std::function<void(const std::string&)> callback) {
  m_onTextChanged = callback;
}

void UITextInput::SetOnSubmit(
    std::function<void(const std::string&)> callback) {
  m_onSubmit = callback;
}

void UITextInput::Focus() {
  if (!m_isFocused) {
    m_isFocused = true;
    m_cursorVisible = true;
    m_cursorBlinkTime = 0.0f;
    SDL_StartTextInput();
  }
}

void UITextInput::Unfocus() {
  if (m_isFocused) {
    m_isFocused = false;
    m_cursorVisible = false;
    SDL_StopTextInput();
  }
}

void UITextInput::Clear() {
  m_text.clear();
  m_cursorPosition = 0;
  m_needsUpdate = true;
  if (m_onTextChanged) {
    m_onTextChanged(m_text);
  }
}

bool UITextInput::LoadFont() {
  if (m_font) {
    return true;
  }

  if (m_fontPath.empty()) {
    const char* defaultFonts[] = {"../Client/assets/Font.ttf"};

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
    std::cerr << "Failed to load font for text input: " << TTF_GetError()
              << std::endl;
    return false;
  }

  return true;
}

std::string UITextInput::GetDisplayText() const {
  if (m_isPassword && !m_text.empty()) {
    return std::string(m_text.length(), '*');
  }
  return m_text;
}

void UITextInput::UpdateTextTexture(SDL_Renderer* renderer) {
  if (!LoadFont()) {
    return;
  }

  if (m_textTexture) {
    SDL_DestroyTexture(m_textTexture);
    m_textTexture = nullptr;
  }

  std::string displayText = m_text.empty() ? m_placeholder : GetDisplayText();
  SDL_Color color = m_text.empty() ? m_placeholderColor : m_textColor;

  if (displayText.empty()) {
    return;
  }

  SDL_Surface* surface =
      TTF_RenderText_Blended(m_font, displayText.c_str(), color);
  if (!surface) {
    std::cerr << "Failed to render text input: " << TTF_GetError() << std::endl;
    return;
  }

  m_textTexture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  m_needsUpdate = false;
}

void UITextInput::Update(float deltaTime) {
  if (m_isFocused) {
    m_cursorBlinkTime += deltaTime;
    if (m_cursorBlinkTime >= 0.5f) {
      m_cursorVisible = !m_cursorVisible;
      m_cursorBlinkTime = 0.0f;
    }
  } else {
    m_cursorVisible = false;
  }
}

void UITextInput::Render(SDL_Renderer* renderer, TextureManager* textures) {
  if (!m_visible) return;

  if (m_needsUpdate) {
    UpdateTextTexture(renderer);
  }

  // Draw background
  SDL_SetRenderDrawColor(renderer, m_backgroundColor.r, m_backgroundColor.g,
                         m_backgroundColor.b, m_backgroundColor.a);
  SDL_RenderFillRect(renderer, &m_rect);

  // Draw border
  SDL_Color borderColor = m_isFocused ? m_focusedBorderColor : m_borderColor;
  SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b,
                         borderColor.a);

  for (int i = 0; i < m_borderThickness; ++i) {
    SDL_Rect borderRect = {m_rect.x - i, m_rect.y - i, m_rect.w + i * 2,
                           m_rect.h + i * 2};
    SDL_RenderDrawRect(renderer, &borderRect);
  }

  // Draw text
  if (m_textTexture) {
    int textW, textH;
    SDL_QueryTexture(m_textTexture, nullptr, nullptr, &textW, &textH);

    int textX = m_rect.x + m_padding;
    int textY = m_rect.y + (m_rect.h - textH) / 2;

    // Clip text to fit inside the input box
    int maxWidth = m_rect.w - m_padding * 2;
    int renderWidth = std::min(textW, maxWidth);

    // If text is wider than the box, show the end (where cursor is)
    int srcX = textW > maxWidth ? textW - maxWidth : 0;
    SDL_Rect srcRect = {srcX, 0, renderWidth, textH};
    SDL_Rect destRect = {textX, textY, renderWidth, textH};

    SDL_RenderCopy(renderer, m_textTexture, &srcRect, &destRect);

    // Draw cursor
    if (m_isFocused && m_cursorVisible && !m_text.empty() && LoadFont()) {
      // Calculate cursor position
      std::string textBeforeCursor =
          GetDisplayText().substr(0, m_cursorPosition);

      int cursorX = textX;
      if (!textBeforeCursor.empty()) {
        int textWidth;
        TTF_SizeText(m_font, textBeforeCursor.c_str(), &textWidth, nullptr);
        cursorX += textWidth - srcX;
      }

      // Only draw cursor if it's within visible area
      if (cursorX >= textX && cursorX <= m_rect.x + m_rect.w - m_padding) {
        SDL_SetRenderDrawColor(renderer, m_textColor.r, m_textColor.g,
                               m_textColor.b, m_textColor.a);
        SDL_Rect cursorRect = {cursorX, textY, 2, textH};
        SDL_RenderFillRect(renderer, &cursorRect);
      }
    } else if (m_isFocused && m_cursorVisible && m_text.empty()) {
      // Draw cursor at start when text is empty
      SDL_SetRenderDrawColor(renderer, m_textColor.r, m_textColor.g,
                             m_textColor.b, m_textColor.a);
      int cursorHeight = m_fontSize;
      int cursorY = m_rect.y + (m_rect.h - cursorHeight) / 2;
      SDL_Rect cursorRect = {textX, cursorY, 2, cursorHeight};
      SDL_RenderFillRect(renderer, &cursorRect);
    }
  } else if (m_isFocused && m_cursorVisible) {
    // Draw cursor even when no text texture exists
    SDL_SetRenderDrawColor(renderer, m_textColor.r, m_textColor.g,
                           m_textColor.b, m_textColor.a);
    int textX = m_rect.x + m_padding;
    int cursorHeight = m_fontSize;
    int cursorY = m_rect.y + (m_rect.h - cursorHeight) / 2;
    SDL_Rect cursorRect = {textX, cursorY, 2, cursorHeight};
    SDL_RenderFillRect(renderer, &cursorRect);
  }
}

bool UITextInput::IsPointInside(int x, int y) const {
  return (x >= m_rect.x && x < m_rect.x + m_rect.w && y >= m_rect.y &&
          y < m_rect.y + m_rect.h);
}

bool UITextInput::HandleEvent(const SDL_Event& event) {
  if (!m_visible || !m_enabled) return false;

  // Handle mouse clicks for focus
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    int x = event.button.x;
    int y = event.button.y;

    bool clickedInside = IsPointInside(x, y);

    if (clickedInside) {
      if (!m_isFocused) {
        Focus();
      }
      return true;
    } else {
      if (m_isFocused) {
        Unfocus();
      }
    }
  }

  // Only handle text input if focused
  if (!m_isFocused) {
    return false;
  }

  // Handle text input
  if (event.type == SDL_TEXTINPUT) {
    for (int i = 0; event.text.text[i] != '\0'; ++i) {
      InsertChar(event.text.text[i]);
    }
    return true;
  }

  // Handle keyboard input
  if (event.type == SDL_KEYDOWN) {
    switch (event.key.keysym.sym) {
      case SDLK_BACKSPACE:
        DeleteChar();
        return true;

      case SDLK_DELETE:
        if (m_cursorPosition < m_text.length()) {
          m_text.erase(m_cursorPosition, 1);
          m_needsUpdate = true;
          if (m_onTextChanged) {
            m_onTextChanged(m_text);
          }
        }
        return true;

      case SDLK_LEFT:
        MoveCursorLeft();
        return true;

      case SDLK_RIGHT:
        MoveCursorRight();
        return true;

      case SDLK_HOME:
        m_cursorPosition = 0;
        m_cursorVisible = true;
        m_cursorBlinkTime = 0.0f;
        return true;

      case SDLK_END:
        m_cursorPosition = m_text.length();
        m_cursorVisible = true;
        m_cursorBlinkTime = 0.0f;
        return true;

      case SDLK_RETURN:
      case SDLK_KP_ENTER:
        if (m_onSubmit) {
          m_onSubmit(m_text);
        }
        return true;

      case SDLK_ESCAPE:
        Unfocus();
        return true;

      case SDLK_v:
        if (SDL_GetModState() & KMOD_CTRL) {
          // Paste from clipboard
          char* clipboard = SDL_GetClipboardText();
          if (clipboard) {
            std::string pasteText(clipboard);
            // Insert clipboard text at cursor
            if (m_text.length() + pasteText.length() <= m_maxLength) {
              m_text.insert(m_cursorPosition, pasteText);
              m_cursorPosition += pasteText.length();
              m_needsUpdate = true;
              m_cursorVisible = true;
              m_cursorBlinkTime = 0.0f;
              if (m_onTextChanged) {
                m_onTextChanged(m_text);
              }
            }
            SDL_free(clipboard);
          }
          return true;
        }
        break;

      case SDLK_c:
        if (SDL_GetModState() & KMOD_CTRL) {
          // Copy to clipboard
          SDL_SetClipboardText(m_text.c_str());
          return true;
        }
        break;

      case SDLK_x:
        if (SDL_GetModState() & KMOD_CTRL) {
          // Cut to clipboard
          SDL_SetClipboardText(m_text.c_str());
          Clear();
          return true;
        }
        break;

      case SDLK_a:
        if (SDL_GetModState() & KMOD_CTRL) {
          // Select all (move cursor to end for now)
          m_cursorPosition = m_text.length();
          m_cursorVisible = true;
          m_cursorBlinkTime = 0.0f;
          return true;
        }
        break;
    }
  }

  return false;
}

void UITextInput::InsertChar(char c) {
  if (m_text.length() >= m_maxLength) {
    return;
  }

  // Only allow printable ASCII characters
  if (c >= 32 && c <= 126) {
    m_text.insert(m_cursorPosition, 1, c);
    m_cursorPosition++;
    m_needsUpdate = true;
    m_cursorVisible = true;
    m_cursorBlinkTime = 0.0f;

    if (m_onTextChanged) {
      m_onTextChanged(m_text);
    }
  }
}

void UITextInput::DeleteChar() {
  if (m_cursorPosition > 0) {
    m_text.erase(m_cursorPosition - 1, 1);
    m_cursorPosition--;
    m_needsUpdate = true;
    m_cursorVisible = true;
    m_cursorBlinkTime = 0.0f;

    if (m_onTextChanged) {
      m_onTextChanged(m_text);
    }
  }
}

void UITextInput::MoveCursorLeft() {
  if (m_cursorPosition > 0) {
    m_cursorPosition--;
    m_cursorVisible = true;
    m_cursorBlinkTime = 0.0f;
  }
}

void UITextInput::MoveCursorRight() {
  if (m_cursorPosition < m_text.length()) {
    m_cursorPosition++;
    m_cursorVisible = true;
    m_cursorBlinkTime = 0.0f;
  }
}
