#pragma once
#include <SDL2/SDL.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "ui/UIElement.hpp"

class RenderingSubsystem;

class UIManager {
private:
    std::vector<std::unique_ptr<UIElement>> m_elements;
    SDL_Renderer* m_renderer;
    RenderingSubsystem* m_renderingSubsystem;
    int m_screenWidth;
    int m_screenHeight;

public:
    UIManager(SDL_Renderer* renderer, RenderingSubsystem* renderSys, int w, int h)
        : m_renderer(renderer),
          m_renderingSubsystem(renderSys),
          m_screenWidth(w),
          m_screenHeight(h) {}

    ~UIManager() {
        Clear();
    }

    template <typename T, typename... Args>
    T* AddElement(Args&&... args) {
        auto element = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = element.get();
        m_elements.push_back(std::move(element));
        return ptr;
    }

    void Clear() { m_elements.clear(); }

    void Update(float deltaTime) {
        for (auto& element : m_elements) {
            if (element->IsVisible()) {
                element->Update(deltaTime);
            }
        }
    }

    void Render() {
        std::sort(m_elements.begin(), m_elements.end(),
                  [](const std::unique_ptr<UIElement>& a,
                     const std::unique_ptr<UIElement>& b) {
                      return a->GetLayer() < b->GetLayer();
                  });

        for (auto& element : m_elements) {
            if (element->IsVisible()) {
                element->Render(m_renderer, m_renderingSubsystem);
            }
        }
    }

    bool HandleEvent(const SDL_Event& event) {
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            if ((*it)->IsVisible() && (*it)->IsEnabled()) {
                if ((*it)->HandleEvent(event)) {
                    return true;
                }
            }
        }
        return false;
    }

    void SetScreenSize(int w, int h) {
        m_screenWidth = w;
        m_screenHeight = h;
    }

    size_t GetElementCount() const { return m_elements.size(); }
    int GetScreenWidth() const { return m_screenWidth; }
    int GetScreenHeight() const { return m_screenHeight; }
};
