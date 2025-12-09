#pragma once
#include <SDL2/SDL.h>
#include <string>
#include "graphics/TextureManager.hpp"

enum class UIAnchor {
  TopLeft, TopCenter, TopRight,
  MiddleLeft, Center, MiddleRight,
  BottomLeft, BottomCenter, BottomRight
};

class UIElement {
protected:
  SDL_Rect m_rect;
  UIAnchor m_anchor;
  bool m_visible;
  bool m_enabled;
  int m_layer;

public:
  UIElement(int x, int y, int w, int h, UIAnchor anchor = UIAnchor::TopLeft)
    : m_rect{x, y, w, h}, 
      m_anchor(anchor), 
      m_visible(true), 
      m_enabled(true), 
      m_layer(0) {}
  
  virtual ~UIElement() = default;

  virtual void Update(float deltaTime) {}
  virtual void Render(SDL_Renderer* renderer, TextureManager* textures) = 0;
  virtual bool HandleEvent(const SDL_Event& event) { return false; }
  
  SDL_Rect GetScreenRect(int screenW, int screenH) const;
  
  void SetVisible(bool visible) { m_visible = visible; }
  bool IsVisible() const { return m_visible; }
  void SetEnabled(bool enabled) { m_enabled = enabled; }
  bool IsEnabled() const { return m_enabled; }
  void SetLayer(int layer) { m_layer = layer; }
  int GetLayer() const { return m_layer; }
  void SetPosition(int x, int y) { m_rect.x = x; m_rect.y = y; }
  void SetSize(int w, int h) { m_rect.w = w; m_rect.h = h; }
  SDL_Rect GetRect() const { return m_rect; }
};
