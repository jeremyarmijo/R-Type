#include "ui/UIElement.hpp"

SDL_Rect UIElement::GetScreenRect(int screenW, int screenH) const {
  SDL_Rect result = m_rect;

  switch (m_anchor) {
    case UIAnchor::TopLeft:
      break;
    case UIAnchor::TopCenter:
      result.x = (screenW - m_rect.w) / 2 + m_rect.x;
      break;
    case UIAnchor::TopRight:
      result.x = screenW - m_rect.w - m_rect.x;
      break;
    case UIAnchor::MiddleLeft:
      result.y = (screenH - m_rect.h) / 2 + m_rect.y;
      break;
    case UIAnchor::Center:
      result.x = (screenW - m_rect.w) / 2 + m_rect.x;
      result.y = (screenH - m_rect.h) / 2 + m_rect.y;
      break;
    case UIAnchor::MiddleRight:
      result.x = screenW - m_rect.w - m_rect.x;
      result.y = (screenH - m_rect.h) / 2 + m_rect.y;
      break;
    case UIAnchor::BottomLeft:
      result.y = screenH - m_rect.h - m_rect.y;
      break;
    case UIAnchor::BottomCenter:
      result.x = (screenW - m_rect.w) / 2 + m_rect.x;
      result.y = screenH - m_rect.h - m_rect.y;
      break;
    case UIAnchor::BottomRight:
      result.x = screenW - m_rect.w - m_rect.x;
      result.y = screenH - m_rect.h - m_rect.y;
      break;
  }

  return result;
}
