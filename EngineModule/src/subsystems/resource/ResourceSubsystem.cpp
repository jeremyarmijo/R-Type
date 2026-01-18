#include "subsystems/resource/ResourceSubsystem.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

ResourceSubsystem::ResourceSubsystem()
    : m_totalMemoryUsed(0),
      m_cacheLimit(100 * 1024 * 1024),  // 100 MB default
      m_isPreloading(false),
      m_preloadProgress(0.0f) {}

ResourceSubsystem::~ResourceSubsystem() { Shutdown(); }

bool ResourceSubsystem::Initialize() {
  std::cout << "Initializing Resource Subsystem..." << std::endl;
  std::cout << "Cache limit: " << (m_cacheLimit / 1024 / 1024) << " MB"
            << std::endl;
  return true;
}

void ResourceSubsystem::Shutdown() {
  std::cout << "Shutting down Resource Subsystem..." << std::endl;
  ClearCache();
}

void ResourceSubsystem::Update(float deltaTime) {
  if (m_isPreloading) {
    UpdatePreloading();
  }

  // Check if we need to evict resources
  if (m_totalMemoryUsed > m_cacheLimit) {
    EvictLRUResources();
  }
}

void ResourceSubsystem::RegisterResource(const std::string& id,
                                         ResourceType type,
                                         const std::string& filepath,
                                         LoadStrategy strategy) {
  m_loadStrategies[id] = strategy;

  if (strategy == LoadStrategy::IMMEDIATE) {
    LoadResource(id);
  } else if (strategy == LoadStrategy::PRELOAD) {
    AddToPreloadQueue(id);
  }

  std::cout << "Registered resource: " << id
            << " (strategy: " << static_cast<int>(strategy) << ")" << std::endl;
}

std::shared_ptr<ResourceHandle> ResourceSubsystem::LoadResource(
    const std::string& id) {
  auto it = m_resources.find(id);
  if (it != m_resources.end()) {
    it->second->refCount++;
    return it->second;
  }

  auto handle = std::make_shared<ResourceHandle>(id, ResourceType::DATA);

  if (LoadResourceImpl(id, *handle)) {
    m_resources[id] = handle;
    m_totalMemoryUsed += handle->memorySize;
    handle->refCount = 1;

    std::cout << "Loaded resource: " << id << " (" << handle->memorySize
              << " bytes)" << std::endl;
    return handle;
  }

  return nullptr;
}

void ResourceSubsystem::UnloadResource(const std::string& id) {
  auto it = m_resources.find(id);
  if (it != m_resources.end()) {
    it->second->refCount--;

    if (it->second->refCount == 0) {
      m_totalMemoryUsed -= it->second->memorySize;
      m_resources.erase(it);
      std::cout << "Unloaded resource: " << id << std::endl;
    }
  }
}

void ResourceSubsystem::UnloadUnusedResources() {
  std::vector<std::string> toRemove;

  for (auto& [id, handle] : m_resources) {
    if (handle->refCount == 0) {
      toRemove.push_back(id);
    }
  }

  for (const auto& id : toRemove) {
    UnloadResource(id);
  }

  std::cout << "Unloaded " << toRemove.size() << " unused resources"
            << std::endl;
}

std::shared_ptr<ResourceHandle> ResourceSubsystem::GetResource(
    const std::string& id) {
  auto it = m_resources.find(id);
  if (it != m_resources.end()) {
    return it->second;
  }

  // On-demand loading
  auto strategyIt = m_loadStrategies.find(id);
  if (strategyIt != m_loadStrategies.end() &&
      strategyIt->second == LoadStrategy::ON_DEMAND) {
    return LoadResource(id);
  }

  return nullptr;
}

void ResourceSubsystem::AddToPreloadQueue(const std::string& id) {
  m_preloadQueue.push_back(id);
}

void ResourceSubsystem::StartPreload() {
  if (m_preloadQueue.empty()) {
    return;
  }

  m_isPreloading = true;
  m_preloadProgress = 0.0f;
  std::cout << "Starting preload of " << m_preloadQueue.size() << " resources"
            << std::endl;
}

void ResourceSubsystem::UpdatePreloading() {
  if (m_preloadQueue.empty()) {
    m_isPreloading = false;
    m_preloadProgress = 1.0f;
    return;
  }

  std::string id = m_preloadQueue.back();
  m_preloadQueue.pop_back();

  LoadResource(id);

  size_t remaining = m_preloadQueue.size();
  size_t total = remaining + 1;
  m_preloadProgress = 1.0f - (static_cast<float>(remaining) / total);
}

void ResourceSubsystem::ClearCache() {
  std::cout << "Clearing resource cache..." << std::endl;
  m_resources.clear();
  m_totalMemoryUsed = 0;
}

bool ResourceSubsystem::LoadResourceImpl(const std::string& id,
                                         ResourceHandle& handle) {
  // Placeholder - actual loading would be done by specific subsystems
  handle.memorySize = 1024;  // Example size
  return true;
}

void ResourceSubsystem::EvictLRUResources() {
  std::vector<std::string> candidates;

  for (auto& [id, handle] : m_resources) {
    if (handle->refCount == 0) {
      candidates.push_back(id);
    }
  }

  // Evict oldest unused resources
  for (const auto& id : candidates) {
    if (m_totalMemoryUsed <= m_cacheLimit * 0.8f) {
      break;
    }
    UnloadResource(id);
  }
}

extern "C" {
ISubsystem* CreateSubsystem() { return new ResourceSubsystem(); }
}
