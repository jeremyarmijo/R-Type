#include "engine/GameEngine.hpp"

#include <SDL_image.h>
#include <SDL_ttf.h>

#include <iostream>
#include <string>

#include "Collision/CollisionController.hpp"
#include "Collision/Items.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/EnemySpawn.hpp"
#include "Player/Projectile.hpp"
/**
 * @brief Creates of the SDL window, intializes the TextureManager class and
 * registers game components in the ECS registry
 *
 * @param title title of the window
 * @param width width of the window
 * @param height height of the window
 * @return true
 * @return false
 */
bool GameEngine::Initialize(const std::string& title, int width, int height) {
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

  if (TTF_Init() == -1) {
    std::cerr << "SDL_ttf initialization failed: " << TTF_GetError()
              << std::endl;
    return false;
  }

  if (!m_audioManager.Initialize()) {
    std::cerr << "Failed to initialize audio!" << std::endl;
  }

  m_window =
      SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

  if (!m_window) {
    std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
    return false;
  }

  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

  if (!m_renderer) {
    std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
    return false;
  }

  // Initialize texture manager with renderer
  m_textureManager = TextureManager(m_renderer);

  RegisterComponents();
  // RegisterSystems(); currently broken

  m_running = true;

  std::cout << "Engine initialized successfully" << std::endl;
  return true;
}

/**
 * @brief connects client to game server
 *
 * @param serverIP
 * @param port
 * @return true
 * @return false
 */
bool GameEngine::ConnectToServer(const std::string& serverIP, int port) {
  std::cout << "Connected to server at " << serverIP << ":" << port
            << std::endl;
  return true;
}

/**
 * @brief registers entity components in the ECS registry
 *
 */
void GameEngine::RegisterComponents() {
  // Physics components
  m_registry.register_component<Transform>();
  m_registry.register_component<RigidBody>();
  m_registry.register_component<BoxCollider>();

  // Render components
  m_registry.register_component<Sprite>();
  m_registry.register_component<Animation>();
  m_registry.register_component<Camera>();

  // Player components
  m_registry.register_component<PlayerEntity>();
  m_registry.register_component<InputState>();

  // Enemy / Gameplay
  m_registry.register_component<Enemy>();
  m_registry.register_component<Boss>();
  m_registry.register_component<Items>();
  m_registry.register_component<Collision>();
  m_registry.register_component<EnemySpawning>();

  // Weapon & Projectile components
  m_registry.register_component<Weapon>();
  m_registry.register_component<Projectile>();

  // Network components

  std::cout << "Components registered" << std::endl;
}

/**
 * @brief registers system functions in the ECS registry (currently broken, DO
 * NOT USE)
 *
 */
void GameEngine::RegisterSystems() {
  // Systems will be called automatically by run_systems()

  // add in order of operation

  /*
  m_registry.add_system<PlayerEntity, Transform, RigidBody>(
      [this](Registry& reg,
             SparseArray<PlayerEntity>& PlayerEntity
             SparseArray<Transform>& transforms,
             SparseArray<RigidBody>& rigidbodies) {
          auto& colliders = reg.get_components<BoxCollider>();
          player_input_system(reg, PlayerEntity, transforms, rigidbodies,
                            colliders, &m_inputManager);
      }
  );

  m_registry.add_system<Animation, Sprite>(
      [this](Registry& reg, SparseArray<Animation>& animations,
  SparseArray<Sprite>& sprites) { animation_system(reg, animations, sprites,
  &m_animationManager, m_deltaTime);
      }
  );

  m_registry.add_system<Transform, RigidBody>(
      [this](Registry& reg, SparseArray<Transform>& transforms,
  SparseArray<RigidBody>& rigidbodies) { physics_movement_system(reg,
  transforms, rigidbodies, m_deltaTime, m_gravity);
      }
  );

  m_registry.add_system<Transform, BoxCollider, RigidBody>(
      [this](Registry& reg,
             const SparseArray<Transform>& transforms,
             const SparseArray<BoxCollider>& colliders,
             SparseArray<RigidBody>& rigidbodies) {
          collision_detection_system(reg, transforms, colliders, rigidbodies);
      }
  );
  */

  std::cout << "Systems registered" << std::endl;
}

/**
 * @brief destroys SDL renderer and window
 *
 */
void GameEngine::Shutdown() {
  if (m_sceneManager) {
    m_sceneManager->ClearAllScenes();
  }

  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
  }

  if (m_window) {
    SDL_DestroyWindow(m_window);
    m_window = nullptr;
  }

  m_audioManager.Shutdown();

  m_networkManager.Disconnect();

  IMG_Quit();
  TTF_Quit();
  SDL_Quit();

  std::cout << "Engine shutdown complete" << std::endl;
}

/**
 * @brief Creates a sprite
 *
 * @param textureKey texture key in the texture manager
 * @param position spawn coordinates
 * @param layer position sprite on different layers for rendering
 * @return Entity
 */
Entity GameEngine::CreateSprite(const std::string& textureKey, Vector2 position,
                                int layer) {
  Entity entity = m_registry.spawn_entity();

  m_registry.emplace_component<Transform>(entity, position, Vector2{1, 1},
                                          0.0f);
  m_registry.emplace_component<Sprite>(entity, textureKey, SDL_Rect{0, 0, 0, 0},
                                       Vector2{0.5f, 0.5f}, layer);
  return entity;
}

/**
 * @brief Creates a sprite with physics components; rigidBody and boxCollider
 *
 * @param textureKey texture key in the texture manager
 * @param position spawn coordinates
 * @param size size of the box collider
 * @param isStatic
 * @return Entity
 */
Entity GameEngine::CreatePhysicsObject(const std::string& textureKey,
                                       Vector2 position, Vector2 size,
                                       bool isStatic) {
  Entity entity = CreateSprite(textureKey, position);

  m_registry.emplace_component<RigidBody>(entity, 1.0f, 0.5f, isStatic);
  m_registry.emplace_component<BoxCollider>(entity, size.x, size.y);

  return entity;
}

Entity GameEngine::CreateAnimatedSprite(const std::string& textureKey,
                                        Vector2 position,
                                        const std::string& animationKey,
                                        int layer) {
  Entity entity = CreateSprite(textureKey, position, layer);

  m_registry.emplace_component<Animation>(entity, animationKey, true);
  auto& sprite = m_registry.get_components<Sprite>()[entity];
  auto& animation = m_registry.get_components<Animation>()[entity];

  const AnimationClip* clip = m_animationManager.GetAnimation(animationKey);

  if (clip && !clip->frames.empty()) {
    animation->currentFrame = 0;
    animation->currentTime = 0;
    animation->isPlaying = true;

    sprite->sourceRect = clip->frames[0].sourceRect;
  } else {
    std::cerr << "CreateAnimatedSprite ERROR: Animation not found: "
              << animationKey << std::endl;
  }

  return entity;
}

Entity GameEngine::CreatePlayer(const std::string& textureKey,
                                const std::string& animationKey,
                                Vector2 position, float moveSpeed) {
  Entity player = CreateAnimatedSprite(textureKey, position, animationKey);

  m_registry.emplace_component<RigidBody>(player, 1.0f, 0.5f, false);
  m_registry.emplace_component<BoxCollider>(player, 60.0f, 32.0f);
  m_registry.emplace_component<PlayerEntity>(player, moveSpeed);

  // Add weapon component so player can shoot
  Weapon playerWeapon = CreateWeapon(5.0f, true);
  m_registry.emplace_component<Weapon>(player, playerWeapon);

  return player;
}

Entity GameEngine::CreateProjectile(const std::string& textureKey,
                                    Vector2 position, Vector2 direction,
                                    float speed, size_t ownerId) {
  Entity projectile = m_registry.spawn_entity();

  m_registry.emplace_component<Transform>(projectile, position, Vector2{1, 1},
                                          0.0f);
  m_registry.emplace_component<Sprite>(
      projectile, textureKey, SDL_Rect{0, 0, 16, 16}, Vector2{0.5f, 0.5f}, 1);
  m_registry.emplace_component<RigidBody>(projectile, 1.0f, 0.0f, false);
  m_registry.emplace_component<BoxCollider>(projectile, 16.0f, 16.0f);
  m_registry.emplace_component<Projectile>(projectile, 10.0f, speed, direction,
                                           5.0f, ownerId);

  return projectile;
}

Weapon GameEngine::CreateWeapon(float fireRate, bool isAutomatic) {
  // Arme automatique simple avec munitions infinies
  return Weapon(fireRate,     // fireRate (projectiles par seconde)
                isAutomatic,  // isAutomatic
                -1,           // maxAmmo (-1 = infini)
                -1,           // magazineSize (-1 = pas de chargeur)
                -1.0f,        // reloadTime (-1 = pas de rechargement)
                false,        // isBurst
                1,            // burstCount
                0.0f);        // burstInterval
}

void GameEngine::Run() {
  Uint32 lastTime = SDL_GetTicks();

  std::cout << "Starting game loop..." << std::endl;

  while (m_running) {
    Uint32 currentTime = SDL_GetTicks();
    m_deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    if (m_deltaTime > 0.05f) {
      m_deltaTime = 0.05f;
    }

    m_inputManager.Update();
    HandleEvents();
    Update(m_deltaTime);
    if (m_sceneManager) {
      m_sceneManager->Update(m_deltaTime);
    }
    Render();
  }

  std::cout << "Game loop ended" << std::endl;
}

void GameEngine::HandleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      m_running = false;
    } else if (event.type == SDL_KEYDOWN) {
      HandleKeyPress(event.key.keysym.sym);
    }
    if (m_sceneManager) {
      m_sceneManager->HandleEvent(event);
    }
  }
}

void GameEngine::HandleKeyPress(SDL_Keycode key) {
  switch (key) {
    case SDLK_ESCAPE:
      m_running = false;
      break;

    case SDLK_F1:
      m_showDebug = !m_showDebug;
      std::cout << "Debug: " << (m_showDebug ? "ON" : "OFF") << std::endl;
      break;

    default:
      break;
  }
}

void GameEngine::Update(float deltaTime) {
  // m_registry.run_systems(); currently broken

  auto& colliders = m_registry.get_components<BoxCollider>();
  auto& playerEntity = m_registry.get_components<PlayerEntity>();
  auto& transforms = m_registry.get_components<Transform>();
  auto& rigidbodies = m_registry.get_components<RigidBody>();
  auto& animations = m_registry.get_components<Animation>();
  auto& sprites = m_registry.get_components<Sprite>();
  auto& weapons = m_registry.get_components<Weapon>();
  auto& projectiles = m_registry.get_components<Projectile>();

  // call systems in order of operation
  player_input_system(m_registry, playerEntity, transforms, rigidbodies,
                      colliders, &m_inputManager, &m_networkManager);
  animation_system(m_registry, animations, sprites, &m_animationManager,
                   deltaTime);
  physics_movement_system(m_registry, transforms, rigidbodies, deltaTime,
                          m_gravity);

  // UpdateCamera();
}

void GameEngine::UpdateCamera() {}

void GameEngine::Render() {
  SDL_SetRenderDrawColor(m_renderer, 50, 50, 80, 255);
  SDL_RenderClear(m_renderer);

  if (m_sceneManager) {
    m_sceneManager->Render();
  }

  if (m_showDebug) RenderDebugInfo();

  SDL_RenderPresent(m_renderer);
}

void GameEngine::RenderDebugInfo() {
  // Show collision boxes
  auto& transforms = m_registry.get_components<Transform>();
  auto& colliders = m_registry.get_components<BoxCollider>();

  SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);

  for (auto&& [transform, collider] : Zipper(transforms, colliders)) {
    SDL_Rect rect = collider.GetRect(transform.position);

    // camera offset
    rect.x -= static_cast<int>(m_cameraPosition.x);
    rect.y -= static_cast<int>(m_cameraPosition.y);

    SDL_RenderDrawRect(m_renderer, &rect);
  }
  return;

  // network debug logic to be implemented here
  // example set up
  SDL_Rect statusRect = {10, 10, 20, 20};
  SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);  // Green = connected
  SDL_RenderFillRect(m_renderer, &statusRect);

  SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
  SDL_RenderDrawRect(m_renderer, &statusRect);
}
