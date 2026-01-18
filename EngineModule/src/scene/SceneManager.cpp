// scene/SceneManager.cpp
#include "scene/SceneManager.hpp"
#include "scene/Scene.hpp"
#include "engine/GameEngine.hpp"
#include <iostream>

SceneManager::SceneManager(GameEngine* engine)
    : m_engine(engine),
      m_currentScene(nullptr),
      m_nextScene(nullptr) {}

SceneManager::~SceneManager() {
    ClearAllScenes();
}

bool SceneManager::LoadSceneModule(const std::string& name, const std::string& soPath) {
    if (m_scenePlugins.find(name) != m_scenePlugins.end()) {
        std::cout << "Scene module already loaded: " << name << std::endl;
        return true;
    }
    
    std::cout << "Loading scene module: " << soPath << std::endl;
    
    try {
        // Use DLLoader to load the scene
        auto loader = std::make_unique<DLLoader<Scene>>(soPath, "CreateScene");
        Scene* scene = loader->getInstance();
        
        if (!scene) {
            std::cerr << "Failed to get scene instance" << std::endl;
            return false;
        }

        scene->SetEngine(m_engine);
        scene->SetSceneManager(this);
        
        std::cout << "Created scene: " << scene->GetName() << std::endl;
        
        // Store the plugin
        m_scenePlugins[name] = {std::move(loader), scene};
        
        std::cout << "Scene module loaded successfully: " << name << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to load scene module: " << e.what() << std::endl;
        return false;
    }
}

void SceneManager::UnloadSceneModule(const std::string& name) {
    auto it = m_scenePlugins.find(name);
    if (it == m_scenePlugins.end()) {
        return;
    }
    
    // Exit scene if it's current
    if (m_currentScene == it->second.instance) {
        m_currentScene->OnExit();
        m_currentScene = nullptr;
    }
    
    std::cout << "Unloading scene module: " << name << std::endl;
    
    // DLLoader destructor will handle cleanup
    m_scenePlugins.erase(it);
}

Scene* SceneManager::FindScene(const std::string& name) {
    auto it = m_scenePlugins.find(name);
    if (it != m_scenePlugins.end()) {
        return it->second.instance;
    }
    return nullptr;
}

void SceneManager::ChangeScene(const std::string& name) {
    Scene* scene = FindScene(name);
    
    if (scene) {
        m_nextScene = scene;
        std::cout << "Scene change requested: " << name << std::endl;
    } else {
        std::cerr << "ERROR: Scene '" << name << "' not found!" << std::endl;
        std::cerr << "Available scenes: ";
        for (const auto& [sceneName, _] : m_scenePlugins) {
            std::cerr << sceneName << " ";
        }
        std::cerr << std::endl;
    }
}

void SceneManager::Update(float deltaTime) {
    // Handle scene transition
    if (m_nextScene && m_nextScene != m_currentScene) {
        std::cout << "\n=== SCENE TRANSITION ===" << std::endl;
        
        try {
            if (m_currentScene) {
                std::cout << "Exiting scene: " << m_currentScene->GetName() << std::endl;
                m_currentScene->OnExit();
            }
            
            std::cout << "Clearing all entities from registry..." << std::endl;
            m_engine->GetRegistry().clear_all_entities();
            
            m_currentScene = m_nextScene;
            std::cout << "Entering scene: " << m_currentScene->GetName() << std::endl;
            m_currentScene->OnEnter();
            m_nextScene = nullptr;
            
            std::cout << "Scene transition complete" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "ERROR during scene transition: " << e.what() << std::endl;
            m_nextScene = nullptr;
        }
        
        std::cout << "========================\n" << std::endl;
    }
    
    // Update current scene
    if (m_currentScene) {
        try {
            m_currentScene->Update(deltaTime);
        } catch (const std::exception& e) {
            std::cerr << "ERROR in scene update: " << e.what() << std::endl;
        }
    }
}

void SceneManager::HandleEvent(SDL_Event& event) {
    if (m_currentScene) {
        try {
            m_currentScene->HandleEvent(event);
        } catch (const std::exception& e) {
            std::cerr << "ERROR in scene event handling: " << e.what() << std::endl;
        }
    }
}

void SceneManager::ClearAllScenes() {
    if (m_currentScene) {
        m_currentScene->OnExit();
        m_currentScene = nullptr;
    }
    
    m_nextScene = nullptr;

    m_scenePlugins.clear();
}

const std::string& SceneManager::GetCurrentSceneName() const {
    static const std::string none = "None";
    return m_currentScene ? m_currentScene->GetName() : none;
}
