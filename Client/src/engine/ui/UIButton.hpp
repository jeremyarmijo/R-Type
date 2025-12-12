#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>
#include "ui/UIElement.hpp"

class UIButton : public UIElement {
private:
  std::string m_text;
  std::string m_textureKey;
  std::string m_fontPath;
  int m_fontSize;
  SDL_Color m_normalColor;
  SDL_Color m_hoverColor;
  SDL_Color m_pressedColor;
  SDL_Color m_disabledColor;
  SDL_Color m_textColor;
  SDL_Color m_textHoverColor;
  std::function<void()> m_onClick;
  bool m_isHovered;
  bool m_isPressed;
  TTF_Font* m_font;
  SDL_Texture* m_textTexture;
  bool m_needsTextUpdate;
  int m_textWidth;
  int m_textHeight;

public:
  UIButton(int x, int y, int w, int h, const std::string& text = "",
           const std::string& fontPath = "", int fontSize = 24)
    : UIElement(x, y, w, h),
      m_text(text),
      m_textureKey(""),
      m_fontPath(fontPath),
      m_fontSize(fontSize),
      m_normalColor{100, 100, 100, 255},
      m_hoverColor{150, 150, 150, 255},
      m_pressedColor{80, 80, 80, 255},
      m_disabledColor{60, 60, 60, 255},
      m_textColor{255, 255, 255, 255},
      m_textHoverColor{255, 255, 255, 255},
      m_isHovered(false),
      m_isPressed(false),
      m_font(nullptr),
      m_textTexture(nullptr),
      m_needsTextUpdate(true),
      m_textWidth(0),
      m_textHeight(0) {}

  virtual ~UIButton();

  // Prevent copying due to raw pointers
  UIButton(const UIButton&) = delete;
  UIButton& operator=(const UIButton&) = delete;

  void SetOnClick(std::function<void()> callback) { m_onClick = callback; }
  void SetTexture(const std::string& key) { m_textureKey = key; }
  void SetText(const std::string& text);
  void SetColors(SDL_Color normal, SDL_Color hover, SDL_Color pressed);
  void SetDisabledColor(SDL_Color color) { m_disabledColor = color; }
  void SetTextColor(SDL_Color normal, SDL_Color hover);
  void SetTextColor(SDL_Color color);
  void SetFontSize(int size);
  
  const std::string& GetText() const { return m_text; }
  bool IsHovered() const { return m_isHovered; }
  bool IsPressed() const { return m_isPressed; }

  void Render(SDL_Renderer* renderer, TextureManager* textures) override;
  bool HandleEvent(const SDL_Event& event) override;

private:
  bool IsPointInside(int x, int y) const;
  bool LoadFont();
  void UpdateTextTexture(SDL_Renderer* renderer);
};
