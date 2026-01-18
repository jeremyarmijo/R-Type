#pragma once
#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include <functional>
#include <string>

#include "ui/UIElement.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "rendering/rendering_export.hpp"

class RENDERING_API UITextInput : public UIElement {
 private:
  std::string m_text;
  std::string m_placeholder;
  std::string m_fontPath;
  int m_fontSize;
  SDL_Color m_textColor;
  SDL_Color m_placeholderColor;
  SDL_Color m_backgroundColor;
  SDL_Color m_borderColor;
  SDL_Color m_focusedBorderColor;
  int m_borderThickness;
  int m_padding;
  size_t m_maxLength;
  bool m_isFocused;
  bool m_isPassword;
  size_t m_cursorPosition;
  float m_cursorBlinkTime;
  bool m_cursorVisible;
  TTF_Font* m_font;
  SDL_Texture* m_textTexture;
  bool m_needsUpdate;
  std::function<void(const std::string&)> m_onTextChanged;
  std::function<void(const std::string&)> m_onSubmit;

 public:
  UITextInput(int x, int y, int w, int h,
              const std::string& placeholder = "Enter text...",
              const std::string& fontPath = "", int fontSize = 20,
              UIAnchor anchor = UIAnchor::TopLeft);

  virtual ~UITextInput();

  // Prevent copying due to raw pointers
  UITextInput(const UITextInput&) = delete;
  UITextInput& operator=(const UITextInput&) = delete;

  // Getters
  const std::string& GetText() const { return m_text; }
  bool IsFocused() const { return m_isFocused; }
  bool IsPassword() const { return m_isPassword; }
  size_t GetMaxLength() const { return m_maxLength; }

  // Setters
  void SetText(const std::string& text);
  void SetPlaceholder(const std::string& placeholder);
  void SetMaxLength(size_t length);
  void SetPassword(bool isPassword);
  void SetTextColor(SDL_Color color);
  void SetPlaceholderColor(SDL_Color color);
  void SetBackgroundColor(SDL_Color color);
  void SetBorderColor(SDL_Color normal, SDL_Color focused);
  void SetBorderThickness(int thickness);
  void SetPadding(int padding);
  void SetFontSize(int size);

  // Callbacks
  void SetOnTextChanged(std::function<void(const std::string&)> callback);
  void SetOnSubmit(std::function<void(const std::string&)> callback);

  // Focus management
  void Focus();
  void Unfocus();
  void Clear();

  // Override base class methods
  void Update(float deltaTime) override;
  void Render(SDL_Renderer* renderer, RenderingSubsystem* renderSys) override;
  bool HandleEvent(const SDL_Event& event) override;

 private:
  bool LoadFont();
  void UpdateTextTexture(SDL_Renderer* renderer);
  void InsertChar(char c);
  void DeleteChar();
  void MoveCursorLeft();
  void MoveCursorRight();
  std::string GetDisplayText() const;
  bool IsPointInside(int x, int y) const;
};
