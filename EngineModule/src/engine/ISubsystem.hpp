#pragma once
#include <SDL2/SDL.h>

#include <string>

#include "ecs/Registry.hpp"

enum class SubsystemType {
  RENDERING,
  PHYSICS,
  AUDIO,
  INPUT,
  RESOURCE,
  MESSAGING,
  NETWORK
};

class ISubsystem {
 public:
  virtual ~ISubsystem() = default;

  virtual bool Initialize() = 0;
  virtual void Shutdown() = 0;
  virtual void Update(float deltaTime) = 0;

  virtual void SetRegistry(Registry* registry) = 0;
  virtual void ProcessEvent(SDL_Event event) = 0;

  virtual const char* GetName() const = 0;
  virtual SubsystemType GetType() const = 0;
  virtual const char* GetVersion() const = 0;
};

extern "C" {
typedef ISubsystem* (*CreateSubsystemFunc)();
typedef void (*DestroySubsystemFunc)(ISubsystem*);
}
