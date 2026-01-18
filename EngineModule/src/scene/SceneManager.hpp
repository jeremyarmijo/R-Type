#pragma once
#include <SDL2/SDL.h>
#include <dlfcn.h>

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "ecs/Registry.hpp"
#include "dynamicLibLoader/DLLoader.hpp"

class GameEngine;
class Scene;
class RenderingSubsystem;

class SceneData {
private:
    std::unordered_map<std::string, std::any> m_data;

public:
    template <typename T>
    void Set(const std::string& key, const T& value) {
        m_data[key] = value;
    }

    template <typename T>
    T Get(const std::string& key, const T& defaultValue = T()) const {
        auto it = m_data.find(key);
        if (it != m_data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                std::cerr << "SceneData: Type mismatch for key '" << key << "'" << std::endl;
            }
        }
        return defaultValue;
    }

    bool Has(const std::string& key) const {
        return m_data.find(key) != m_data.end();
    }

    void Clear() { m_data.clear(); }
    void Remove(const std::string& key) { m_data.erase(key); }
};

struct ScenePlugin {
    std::unique_ptr<DLLoader<Scene>> loader;
    Scene* instance;
};

class SceneManager {
private:
    GameEngine* m_engine;

    std::unordered_map<std::string, ScenePlugin> m_scenePlugins;
    
    Scene* m_currentScene;
    Scene* m_nextScene;
    SceneData m_sceneData;

public:
    explicit SceneManager(GameEngine* engine);
    ~SceneManager();

    bool LoadSceneModule(const std::string& name, const std::string& soPath);
    void UnloadSceneModule(const std::string& name);
    
    void ChangeScene(const std::string& name);
    void Update(float deltaTime);
    void HandleEvent(SDL_Event& event);
    
    void ClearAllScenes();
    
    Scene* GetCurrentScene() { return m_currentScene; }
    SceneData& GetSceneData() { return m_sceneData; }
    
    const std::string& GetCurrentSceneName() const;

private:
    Scene* FindScene(const std::string& name);
};
