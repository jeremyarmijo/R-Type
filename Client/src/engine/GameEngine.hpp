#pragma once
#include <SDL2/SDL.h>

#include <string>

#include "AnimationManager.hpp"
#include "InputSystem.hpp"
#include "NetworkManager.hpp"
#include "Physics2D.hpp"
#include "PhysicsSystem.hpp"
#include "Registry.hpp"
#include "RenderComponents.hpp"
#include "RenderSystem.hpp"
#include "TextureManager.hpp"

constexpr int MAX_PLAYERS = 4;

class GameEngine {
 private:
  SDL_Window* m_window;
  SDL_Renderer* m_renderer;
  bool m_running;

  int m_windowWidth;
  int m_windowHeight;

  Registry m_registry;
  TextureManager m_textureManager;
  AnimationManager m_animationManager;
  NetworkManager m_networkManager;
  InputManager m_inputManager;

  Vector2 m_cameraPosition;
  Vector2 m_gravity;
  float m_deltaTime;

  bool m_showDebug;

 public:
  GameEngine()
      : m_window(nullptr),
        m_renderer(nullptr),
        m_running(false),
        m_windowWidth(800),
        m_windowHeight(600),
        m_textureManager(nullptr),
        m_cameraPosition{0, 0},
        m_gravity{0, 0},
        m_deltaTime(0.0f),
        m_showDebug(false) {}

  ~GameEngine() {}

  bool Initialize(const std::string& title, int width, int height);
  bool ConnectToServer(const std::string& serverIP, int port);
  void RegisterComponents();
  void RegisterSystems();
  void Shutdown();
  void Run();

  Entity CreateSprite(const std::string& textureKey, Vector2 position,
                      int layer = 0);
  Entity CreatePhysicsObject(const std::string& textureKey, Vector2 position,
                             Vector2 size, bool isStatic = false);
  Entity CreateAnimatedSprite(const std::string& textureKey, Vector2 position,
                              const std::string& animationKey);
  Entity CreatePlayer(const std::string& textureKey,
                      const std::string& animationKey, Vector2 position,
                      float moveSpeed);

  TextureManager& GetTextureManager() { return m_textureManager; }
  AnimationManager& GetAnimationManager() { return m_animationManager; }
  Registry& GetRegistry() { return m_registry; }
  NetworkManager& GetNetworkManager() { return m_networkManager; }
  InputManager& GetInputManager() { return m_inputManager; }
  SDL_Renderer* GetRenderer() { return m_renderer; }

  Vector2 GetCameraPosition() const { return m_cameraPosition; }
  void SetCameraPosition(Vector2 pos) { m_cameraPosition = pos; }

  Vector2 GetGravity() const { return m_gravity; }
  void SetGravity(Vector2 gravity) { m_gravity = gravity; }

  bool IsConnected() const { return m_networkManager.IsConnected(); }

 private:
  void HandleEvents();
  void HandleKeyPress(SDL_Keycode key);
  void Update(float deltaTime);
  void UpdateCamera();
  void Render();
  void RenderDebugInfo();
};
