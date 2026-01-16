#include "engine/GameEngine.hpp"
#include <iostream>

#include "input/InputSubsystem.hpp"
#include "physics/Physics2D.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "Collision/CollisionController.hpp"
#include "Collision/Items.hpp"
#include "Player/Boss.hpp"
#include "Player/Enemy.hpp"
#include "Player/EnemySpawn.hpp"
#include "Player/Projectile.hpp"

GameEngine::GameEngine() 
    : m_running(false),
      m_deltaTime(0.0f),
      m_windowWidth(800),
      m_windowHeight(600) {
    
    m_sceneManager = std::make_unique<SceneManager>(this);
}

GameEngine::~GameEngine() {
    Shutdown();
}

bool GameEngine::Initialize(const std::string& title, int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    
    std::cout << "=== INITIALIZING GAME ENGINE ===" << std::endl;
    std::cout << "Window: " << width << "x" << height << std::endl;
    
    // Register core components

//   // Player components
//   m_registry.register_component<PlayerEntity>();
//   m_registry.register_component<InputState>();

//   // Enemy / Gameplay
//   m_registry.register_component<Enemy>();
//   m_registry.register_component<Boss>();
//   m_registry.register_component<Items>();
//   m_registry.register_component<Collision>();
//   m_registry.register_component<EnemySpawning>();

//   // Weapon & Projectile components
//   m_registry.register_component<Weapon>();
//   m_registry.register_component<Projectile>();
    
    std::cout << "Core components registered" << std::endl;
    std::cout << "Engine initialized - waiting for subsystems" << std::endl;
    
    return true;
}

bool GameEngine::LoadSubsystem(SubsystemType type, const std::string& pluginPath) {
    if (HasSubsystem(type)) {
        std::cerr << "Subsystem already loaded: " << static_cast<int>(type) << std::endl;
        return false;
    }
    
    std::cout << "Loading subsystem plugin: " << pluginPath << std::endl;
    
    try {
        // Use DLLoader to load the subsystem
        auto loader = std::make_unique<DLLoader<ISubsystem>>(pluginPath, "CreateSubsystem");
        ISubsystem* subsystem = loader->getInstance();
        
        if (!subsystem) {
            std::cerr << "Failed to get subsystem instance" << std::endl;
            return false;
        }
        
        std::cout << "Created subsystem: " << subsystem->GetName() 
                  << " v" << subsystem->GetVersion() << std::endl;
        
        if (!subsystem->Initialize()) {
            std::cerr << "Failed to initialize subsystem" << std::endl;
            return false;
        }

        subsystem->SetRegistry(&m_registry);
    
        m_subsystems[type] = {std::move(loader), subsystem};
        m_updateOrder.push_back(type);
        
        std::cout << "Subsystem loaded successfully: " << subsystem->GetName() << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to load subsystem: " << e.what() << std::endl;
        return false;
    }
}

void GameEngine::UnloadSubsystem(SubsystemType type) {
    auto it = m_subsystems.find(type);
    if (it == m_subsystems.end()) {
        return;
    }
    
    std::cout << "Unloading subsystem: " << it->second.instance->GetName() << std::endl;
    
    it->second.instance->Shutdown();

    m_subsystems.erase(it);
    
    auto orderIt = std::find(m_updateOrder.begin(), m_updateOrder.end(), type);
    if (orderIt != m_updateOrder.end()) {
        m_updateOrder.erase(orderIt);
    }
}

ISubsystem* GameEngine::GetSubsystem(SubsystemType type) {
    auto it = m_subsystems.find(type);
    return (it != m_subsystems.end()) ? it->second.instance : nullptr;
}

bool GameEngine::HasSubsystem(SubsystemType type) const {
    return m_subsystems.find(type) != m_subsystems.end();
}

void GameEngine::Run() {
    m_running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    std::cout << "Starting main loop..." << std::endl;
    
    while (m_running) {
        Uint32 currentTime = SDL_GetTicks();
        m_deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        if (m_deltaTime > 0.05f) {
            m_deltaTime = 0.05f;
        }
        
        HandleEvents();
        Update(m_deltaTime);
    }
    
    std::cout << "Main loop ended" << std::endl;
}

void GameEngine::Update(float deltaTime) {
    for (SubsystemType type : m_updateOrder) {
        auto it = m_subsystems.find(type);
        if (it != m_subsystems.end()) {
            it->second.instance->Update(deltaTime);
        }
    }

    if (m_sceneManager) {
        m_sceneManager->Update(deltaTime);
    }
}

void GameEngine::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
        }

        m_subsystems[SubsystemType::INPUT].instance->ProcessEvent(event);

        if (m_sceneManager) {
            m_sceneManager->HandleEvent(event);
        }
    }
}

void GameEngine::Shutdown() {
    std::cout << "=== SHUTTING DOWN ENGINE ===" << std::endl;
    
    if (m_sceneManager) {
        m_sceneManager->ClearAllScenes();
    }

    for (auto it = m_updateOrder.rbegin(); it != m_updateOrder.rend(); ++it) {
        UnloadSubsystem(*it);
    }
    
    std::cout << "Engine shutdown complete" << std::endl;
}
