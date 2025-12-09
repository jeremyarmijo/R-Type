#pragma once
#include <SDL2/SDL.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "graphics/TextureManager.hpp"
#include "ui/UIElement.hpp"

class UIManager {
 private:
  std::vector<std::unique_ptr<UIElement>> m_elements;
  SDL_Renderer* m_renderer;
  TextureManager* m_textures;
  int m_screenWidth;
  int m_screenHeight;

 public:
  UIManager(SDL_Renderer* renderer, TextureManager* textures, int w, int h)
      : m_renderer(renderer),
        m_textures(textures),
        m_screenWidth(w),
        m_screenHeight(h) {}

  ~UIManager() { Clear(); }

  // Add a new UI element
  template <typename T, typename... Args>
  T* AddElement(Args&&... args) {
    auto element = std::make_unique<T>(std::forward<Args>(args)...);
    T* ptr = element.get();
    m_elements.push_back(std::move(element));
    return ptr;
  }

  // Remove all elements
  void Clear() { m_elements.clear(); }

  // Update all elements
  void Update(float deltaTime) {
    for (auto& element : m_elements) {
      if (element->IsVisible()) {
        element->Update(deltaTime);
      }
    }
  }

  // Render all elements (sorted by layer)
  void Render() {
    // Sort by layer before rendering
    std::sort(m_elements.begin(), m_elements.end(),
              [](const std::unique_ptr<UIElement>& a,
                 const std::unique_ptr<UIElement>& b) {
                return a->GetLayer() < b->GetLayer();
              });

    for (auto& element : m_elements) {
      if (element->IsVisible()) {
        element->Render(m_renderer, m_textures);
      }
    }
  }

  // Handle events (returns true if event was consumed)
  bool HandleEvent(const SDL_Event& event) {
    // Process in reverse order (top layer first) for proper event handling
    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
      if ((*it)->IsVisible() && (*it)->IsEnabled()) {
        if ((*it)->HandleEvent(event)) {
          return true;  // Event consumed
        }
      }
    }
    return false;
  }

  // Update screen dimensions
  void SetScreenSize(int w, int h) {
    m_screenWidth = w;
    m_screenHeight = h;
  }

  // Get number of elements
  size_t GetElementCount() const { return m_elements.size(); }

  // Get screen dimensions
  int GetScreenWidth() const { return m_screenWidth; }
  int GetScreenHeight() const { return m_screenHeight; }
};
