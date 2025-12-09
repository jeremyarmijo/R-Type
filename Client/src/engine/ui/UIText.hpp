#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "ui/UIElement.hpp"

enum class TextAlign {
  Left,
  Center,
  Right
};

class UIText : public UIElement {
private:
  std::string m_text;
  std::string m_fontPath;
  int m_fontSize;
  SDL_Color m_color;
  TextAlign m_alignment;
  TTF_Font* m_font;
  SDL_Texture* m_textTexture;
  bool m_needsUpdate;
  int m_textWidth;
  int m_textHeight;

public:
  UIText(int x, int y, const std::string& text, 
         const std::string& fontPath = "", 
         int fontSize = 24,
         SDL_Color color = {255, 255, 255, 255},
         UIAnchor anchor = UIAnchor::TopLeft);
  
  virtual ~UIText();

  // Prevent copying due to raw pointers
  UIText(const UIText&) = delete;
  UIText& operator=(const UIText&) = delete;

  void SetText(const std::string& text);
  void SetColor(SDL_Color color);
  void SetFontSize(int size);
  void SetAlignment(TextAlign align);

  const std::string& GetText() const { return m_text; }
  int GetTextWidth() const { return m_textWidth; }
  int GetTextHeight() const { return m_textHeight; }

  void Update(float deltaTime) override;
  void Render(SDL_Renderer* renderer, TextureManager* textures) override;

private:
  bool LoadFont();
  void UpdateTextTexture(SDL_Renderer* renderer);
};
