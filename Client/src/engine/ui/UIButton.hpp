#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include "ui/UIElement.hpp"

class UIButton : public UIElement {
private:
  std::string m_text;
  std::string m_textureKey;
  SDL_Color m_normalColor;
  SDL_Color m_hoverColor;
  SDL_Color m_pressedColor;
  SDL_Color m_textColor;
  std::function<void()> m_onClick;
  bool m_isHovered;
  bool m_isPressed;

public:
  UIButton(int x, int y, int w, int h, const std::string& text = "")
    : UIElement(x, y, w, h),
      m_text(text),
      m_textureKey(""),
      m_normalColor{100, 100, 100, 255},
      m_hoverColor{150, 150, 150, 255},
      m_pressedColor{80, 80, 80, 255},
      m_textColor{255, 255, 255, 255},
      m_isHovered(false),
      m_isPressed(false) {}

  void SetOnClick(std::function<void()> callback) { m_onClick = callback; }
  void SetTexture(const std::string& key) { m_textureKey = key; }
  void SetText(const std::string& text) { m_text = text; }
  void SetColors(SDL_Color normal, SDL_Color hover, SDL_Color pressed) {
    m_normalColor = normal;
    m_hoverColor = hover;
    m_pressedColor = pressed;
  }
  void SetTextColor(SDL_Color color) { m_textColor = color; }
  
  bool IsHovered() const { return m_isHovered; }
  bool IsPressed() const { return m_isPressed; }

  void Render(SDL_Renderer* renderer, TextureManager* textures) override;
  bool HandleEvent(const SDL_Event& event) override;

private:
  bool IsPointInside(int x, int y) const;
};
