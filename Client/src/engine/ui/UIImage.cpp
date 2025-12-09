#include "ui/UIImage.hpp"
#include <iostream>

void UIImage::Render(SDL_Renderer* renderer, TextureManager* textures) {
  if (!m_visible) return;
  
  if (m_textureKey.empty()) {
    // Draw placeholder
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &m_rect);
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_RenderDrawRect(renderer, &m_rect);
    return;
  }
  
  if (!textures) {
    std::cerr << "UIImage: TextureManager is null" << std::endl;
    return;
  }
  
  SDL_Texture* texture = textures->GetTexture(m_textureKey);
  if (!texture) {
    std::cerr << "UIImage: Texture not found: " << m_textureKey << std::endl;
    
    // Render placeholder (magenta for missing texture)
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_RenderFillRect(renderer, &m_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &m_rect);
    return;
  }
  
  SDL_Rect sourceRect = m_sourceRect;
  
  // If source rect not set, use full texture
  if (sourceRect.w == 0 || sourceRect.h == 0) {
    int w, h;
    textures->GetTextureSize(m_textureKey, w, h);
    sourceRect = {0, 0, w, h};
  }
  
  SDL_Rect destRect = m_rect;
  
  // Maintain aspect ratio if requested
  if (m_maintainAspectRatio && sourceRect.w > 0 && sourceRect.h > 0) {
    float srcAspect = static_cast<float>(sourceRect.w) / sourceRect.h;
    float dstAspect = static_cast<float>(m_rect.w) / m_rect.h;
    
    if (srcAspect > dstAspect) {
      // Image is wider - fit to width
      int newHeight = static_cast<int>(m_rect.w / srcAspect);
      int yOffset = (m_rect.h - newHeight) / 2;
      destRect = {m_rect.x, m_rect.y + yOffset, m_rect.w, newHeight};
    } else {
      // Image is taller - fit to height
      int newWidth = static_cast<int>(m_rect.h * srcAspect);
      int xOffset = (m_rect.w - newWidth) / 2;
      destRect = {m_rect.x + xOffset, m_rect.y, newWidth, m_rect.h};
    }
  }
  
  SDL_RenderCopy(renderer, texture, &sourceRect, &destRect);
}
