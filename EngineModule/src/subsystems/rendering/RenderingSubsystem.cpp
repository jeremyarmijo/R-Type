#include "rendering/RenderingSubsystem.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>

#include "components/TileMap.hpp"
#include "ecs/Zipper.hpp"
#include "ui/UIElement.hpp"
#include "ui/UIManager.hpp"

RenderingSubsystem::RenderingSubsystem()
    : m_window(nullptr),
      m_renderer(nullptr),
      m_cameraPosition{0, 0},
      m_registry(nullptr),
      m_windowWidth(800),
      m_windowHeight(600) {}

RenderingSubsystem::~RenderingSubsystem() { Shutdown(); }

bool RenderingSubsystem::Initialize() {
  std::cout << "Initializing Rendering Subsystem..." << std::endl;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
    return false;
  }

  if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) &
        (IMG_INIT_PNG | IMG_INIT_JPG))) {
    std::cerr << "SDL_image initialization failed: " << IMG_GetError()
              << std::endl;
    return false;
  }

  if (TTF_Init() == -1) {
    std::cerr << "SDL_ttf initialization failed: " << TTF_GetError()
              << std::endl;
    return false;
  }

  m_window = SDL_CreateWindow("Game Engine", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, m_windowWidth,
                              m_windowHeight, SDL_WINDOW_SHOWN);

  if (!m_window) {
    std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
    return false;
  }

  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

  if (!m_renderer) {
    std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
    return false;
  }

  m_uiManager = std::make_unique<UIManager>(m_renderer, this, m_windowWidth,
                                            m_windowHeight);

  std::cout << "Rendering subsystem initialized" << std::endl;
  return true;
}

void RenderingSubsystem::SetRegistry(Registry* registry) {
  m_registry = registry;
  m_registry->register_component<Sprite>();
  m_registry->register_component<Animation>();
  m_registry->register_component<Camera>();
  m_registry->register_component<TileMap>();
}

void RenderingSubsystem::RenderTileMap() {
  auto& tilemaps = m_registry->get_components<TileMap>();

  for (auto& tilemap : tilemaps) {
    if (!tilemap.has_value() || !tilemap->isLoaded) continue;

    SDL_Renderer* renderer = GetRenderer();

    int startTileX =
        static_cast<int>(tilemap->scrollOffset / tilemap->tileSize);
    int endTileX = startTileX + (800 / tilemap->tileSize) + 2;

    for (int y = 0; y < static_cast<int>(tilemap->height); ++y) {
      for (int x = startTileX;
           x < endTileX && x < static_cast<int>(tilemap->width); ++x) {
        TileType type = tilemap->getTile(x, y);

        if (type == TileType::EMPTY) continue;

        int screenX =
            static_cast<int>(x * tilemap->tileSize - tilemap->scrollOffset);
        int screenY = y * tilemap->tileSize;

        switch (type) {
          case TileType::GROUND:
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
            break;
          case TileType::WALL:
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            break;
          case TileType::CEILING:
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            break;
          case TileType::PLATFORM:
            SDL_SetRenderDrawColor(renderer, 160, 82, 45, 255);
            break;
          default:
            continue;
        }

        SDL_Rect tileRect = {screenX, screenY,
                             static_cast<int>(tilemap->tileSize),
                             static_cast<int>(tilemap->tileSize)};

        SDL_RenderFillRect(renderer, &tileRect);
      }
    }
  }
}

void RenderingSubsystem::Shutdown() {
  std::cout << "Shutting down Rendering Subsystem..." << std::endl;

  m_uiManager.reset();

  for (auto& [key, texture] : m_textures) {
    SDL_DestroyTexture(texture);
  }
  m_textures.clear();
  m_animations.clear();

  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
  }

  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void RenderingSubsystem::Update(float deltaTime) {
  SDL_SetRenderDrawColor(m_renderer, 50, 50, 80, 255);
  SDL_RenderClear(m_renderer);

  if (m_registry) {
    UpdateAnimations(deltaTime);
    RenderSprites();
  }

  if (m_uiManager) {
    m_uiManager->Update(deltaTime);
    m_uiManager->Render();
  }

  SDL_RenderPresent(m_renderer);
}

void RenderingSubsystem::UpdateAnimations(float deltaTime) {
  auto& animations = m_registry->get_components<Animation>();
  auto& sprites = m_registry->get_components<Sprite>();

  for (size_t i = 0; i < std::min(animations.size(), sprites.size()); ++i) {
    if (!animations[i].has_value() || !sprites[i].has_value()) {
      continue;
    }

    auto& anim = animations[i].value();
    auto& sprite = sprites[i].value();

    if (!anim.isPlaying) continue;

    const AnimationClip* clip = GetAnimation(anim.animationKey);
    if (!clip || clip->frames.empty()) continue;

    anim.currentTime += deltaTime;

    const AnimationFrame& currentFrameData = clip->frames[anim.currentFrame];

    if (anim.currentTime >= currentFrameData.duration) {
      anim.currentTime = 0.0f;
      anim.currentFrame++;

      if (anim.currentFrame >= static_cast<int>(clip->frames.size())) {
        if (anim.loop) {
          anim.currentFrame = 0;
        } else {
          anim.currentFrame = static_cast<int>(clip->frames.size()) - 1;
          anim.isPlaying = false;
        }
      }

      if (anim.currentFrame < static_cast<int>(clip->frames.size())) {
        sprite.sourceRect = clip->frames[anim.currentFrame].sourceRect;
      }
    }
  }
}

void RenderingSubsystem::RenderSprites() {
  auto& transforms = m_registry->get_components<Transform>();
  auto& sprites = m_registry->get_components<Sprite>();

  struct RenderData {
    const Transform* transform;
    const Sprite* sprite;
  };

  std::vector<RenderData> renderList;

  for (size_t i = 0; i < std::min(transforms.size(), sprites.size()); ++i) {
    if (transforms[i].has_value() && sprites[i].has_value()) {
      renderList.push_back({&transforms[i].value(), &sprites[i].value()});
    }
  }

  // Sort by layer
  std::sort(renderList.begin(), renderList.end(),
            [](const RenderData& a, const RenderData& b) {
              return a.sprite->layer < b.sprite->layer;
            });

  for (const auto& data : renderList) {
    if (!data.sprite->visible) continue;

    SDL_Texture* texture = GetTexture(data.sprite->textureKey);
    if (!texture) continue;

    SDL_Rect sourceRect = data.sprite->sourceRect;
    if (sourceRect.w == 0 || sourceRect.h == 0) {
      SDL_QueryTexture(texture, nullptr, nullptr, &sourceRect.w, &sourceRect.h);
    }

    SDL_Rect destRect = {
        static_cast<int>(
            data.transform->position.x - m_cameraPosition.x -
            (sourceRect.w * data.transform->scale.x * data.sprite->pivot.x)),
        static_cast<int>(
            data.transform->position.y - m_cameraPosition.y -
            (sourceRect.h * data.transform->scale.y * data.sprite->pivot.y)),
        static_cast<int>(sourceRect.w * data.transform->scale.x),
        static_cast<int>(sourceRect.h * data.transform->scale.y)};

    SDL_RenderCopyEx(m_renderer, texture, &sourceRect, &destRect,
                     data.transform->rotation, nullptr, SDL_FLIP_NONE);
  }
}

bool RenderingSubsystem::LoadTexture(const std::string& key,
                                     const std::string& filepath) {
  if (m_textures.find(key) != m_textures.end()) {
    std::cout << "Texture already loaded: " << key << std::endl;
    return true;
  }

  SDL_Surface* surface = IMG_Load(filepath.c_str());
  if (!surface) {
    std::cerr << "Failed to load texture: " << filepath << " - "
              << IMG_GetError() << std::endl;
    return false;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
  SDL_FreeSurface(surface);

  if (!texture) {
    std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    return false;
  }

  m_textures[key] = texture;
  std::cout << "Loaded texture: " << key << std::endl;
  return true;
}

SDL_Texture* RenderingSubsystem::GetTexture(const std::string& key) {
  auto it = m_textures.find(key);
  return (it != m_textures.end()) ? it->second : nullptr;
}

void RenderingSubsystem::UnloadTexture(const std::string& key) {
  auto it = m_textures.find(key);
  if (it != m_textures.end()) {
    SDL_DestroyTexture(it->second);
    m_textures.erase(it);
    std::cout << "Unloaded texture: " << key << std::endl;
  }
}

void RenderingSubsystem::GetTextureSize(const std::string& key, int& width,
                                        int& height) {
  SDL_Texture* texture = GetTexture(key);
  if (texture) {
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
  }
}

void RenderingSubsystem::CreateAnimation(
    const std::string& name, const std::string& textureKey,
    const std::vector<AnimationFrame>& frames, bool loop) {
  AnimationClip clip(loop);
  clip.textureKey = textureKey;
  clip.frames = frames;
  m_animations[name] = clip;
  std::cout << "Created animation: " << name << " (" << frames.size()
            << " frames)" << std::endl;
}

const AnimationClip* RenderingSubsystem::GetAnimation(
    const std::string& name) const {
  auto it = m_animations.find(name);
  return (it != m_animations.end()) ? &it->second : nullptr;
}

void RenderingSubsystem::RemoveAnimation(const std::string& name) {
  m_animations.erase(name);
}

void RenderingSubsystem::SetWindowSize(int width, int height) {
  m_windowWidth = width;
  m_windowHeight = height;
  if (m_window) {
    SDL_SetWindowSize(m_window, width, height);
  }
  if (m_uiManager) {
    m_uiManager->SetScreenSize(width, height);
  }
}

void RenderingSubsystem::SetWindowTitle(const std::string& title) {
  if (m_window) {
    SDL_SetWindowTitle(m_window, title.c_str());
  }
}

extern "C" {
ISubsystem* CreateSubsystem() { return new RenderingSubsystem(); }
}
