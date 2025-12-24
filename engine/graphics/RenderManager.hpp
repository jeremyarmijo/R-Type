#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>
#include <iostream>

#include "components/Physics2D.hpp"

class RenderManager {
 private:
  SDL_Window* m_window;
  SDL_Renderer* m_renderer;

  int m_windowWidth;
  int m_windowHeight;

  Vector2 m_cameraPosition;

 public:
  RenderManager() 
      : m_window(nullptr),
        m_renderer(nullptr),
        m_windowWidth(800),
        m_windowHeight(600),
        m_cameraPosition{0, 0} {}

  ~RenderManager() { Shutdown(); }

  bool Initialize(const std::string& title, int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
      return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
      std::cerr << "SDL_image initialization failed: " << IMG_GetError()
                << std::endl;
      return false;
    }

    m_window =
    SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                      SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    if (!m_window) {
      std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
      return false;
    }

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED);

    if (!m_renderer) {
      std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
      return false;
    }
  }
  bool Shutdown() {
    if (m_renderer) {
      SDL_DestroyRenderer(m_renderer);
      m_renderer = nullptr;
    }

    if (m_window) {
      SDL_DestroyWindow(m_window);
      m_window = nullptr;
    }
    IMG_Quit();
    SDL_Quit();
  }

  SDL_Renderer* GetRenderer() { return m_renderer; }
};
