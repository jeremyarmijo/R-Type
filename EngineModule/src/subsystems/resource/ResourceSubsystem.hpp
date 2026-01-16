#pragma once
#include <any>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/ISubsystem.hpp"

enum class ResourceType {
    TEXTURE,
    SOUND,
    MUSIC,
    FONT,
    DATA
};

enum class LoadStrategy {
    IMMEDIATE,    // Load right away
    PRELOAD,      // Load during loading screen
    ON_DEMAND,    // Load when first requested
    STREAMING     // Load progressively
};

struct ResourceHandle {
    std::string id;
    ResourceType type;
    std::any data;
    size_t refCount;
    size_t memorySize;
    
    ResourceHandle(const std::string& id, ResourceType type)
        : id(id), type(type), refCount(0), memorySize(0) {}
};

class ResourceSubsystem : public ISubsystem {
private:
    std::unordered_map<std::string, std::shared_ptr<ResourceHandle>> m_resources;
    std::unordered_map<std::string, LoadStrategy> m_loadStrategies;
    
    size_t m_totalMemoryUsed;
    size_t m_cacheLimit;
    
    std::vector<std::string> m_preloadQueue;
    bool m_isPreloading;
    float m_preloadProgress;

public:
    ResourceSubsystem();
    ~ResourceSubsystem() override;
    
    bool Initialize() override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    
    const char* GetName() const override { return "Resource"; }
    SubsystemType GetType() const override { return SubsystemType::RESOURCE; }
    const char* GetVersion() const override { return "1.0.0"; }

    void ProcessEvent(SDL_Event event) override {}

    void SetRegistry(Registry* registry) override {}
    
    void RegisterResource(const std::string& id, ResourceType type, 
                         const std::string& filepath, LoadStrategy strategy = LoadStrategy::ON_DEMAND);
    
    std::shared_ptr<ResourceHandle> LoadResource(const std::string& id);
    void UnloadResource(const std::string& id);
    void UnloadUnusedResources();
    
    std::shared_ptr<ResourceHandle> GetResource(const std::string& id);
    
    void AddToPreloadQueue(const std::string& id);
    void StartPreload();
    bool IsPreloading() const { return m_isPreloading; }
    float GetPreloadProgress() const { return m_preloadProgress; }
    
    void SetCacheLimit(size_t bytes) { m_cacheLimit = bytes; }
    size_t GetMemoryUsed() const { return m_totalMemoryUsed; }
    void ClearCache();

private:
    bool LoadResourceImpl(const std::string& id, ResourceHandle& handle);
    void UpdatePreloading();
    void EvictLRUResources();
};

extern "C" {
    ISubsystem* CreateSubsystem();
}
