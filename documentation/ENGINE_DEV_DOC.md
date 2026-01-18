# Game Engine Developer Documentation

## Overview

- [**Game Engine**](#game-engine)
- [**ECS**](#ecs)
- [**Scene Management**](#scene-management)
- [**Subsystems**](#subsystems)
  - [**Rendering Subsystem**](#rendering-subsystem)
  - [**Physics Subsystem**](#physics-subsystem)
  - [**Input Subsystem**](#input-subsystem)
  - [**Audio Subsystem**](#audio-subsystem)
  - [**Network Subsystem**](#network-subsystem)
  - [**Resource Subsystem**](#resource-subsystem)
  - [**Messaging Subsystem**](#messaging-subsystem)

---

# Game Engine

The `GameEngine` class serves as the core runtime manager, orchestrating subsystems, managing the ECS registry, and controlling the main game loop. It uses a plugin-based architecture where subsystems are loaded dynamically.

---

## Responsibilities

### Core Engine Lifecycle

* Initialize and manage the ECS `Registry`.
* Load and unload subsystems dynamically via `DLLoader`.
* Manage the `SceneManager` for scene transitions.
* Execute the main game loop with delta-time calculation.
* Process SDL events and distribute them to subsystems and scenes.

### Plugin Architecture

* Subsystems are loaded as dynamic libraries (.so/.dll).
* Each subsystem implements the `ISubsystem` interface.
* Subsystems are identified by `SubsystemType` enum.
* Update order is maintained via `m_updateOrder` vector.

### Scene Management

* Owns a `SceneManager` instance.
* Provides scene loading, changing, and data persistence.
* Scenes are also loaded as plugins.

---

## Main Loop Overview

### `Run()`

The main game loop:

1. Calculate frame delta-time (capped at 0.05s).
2. Process input events (`HandleEvents()`).
3. Update all subsystems and current scene (`Update()`).
4. Loop until `m_running` is false.

### `Update(deltaTime)`

* Calls `Update()` on each subsystem in registration order.
* Updates the `SceneManager` which manages scene transitions and updates the current scene.

### `HandleEvents()`

* Polls SDL events.
* Sends events to the `INPUT` subsystem.
* Forwards events to the current scene via `SceneManager`.
* Handles SDL_QUIT to stop the engine.

---

## Component Registration

The engine registers core gameplay components during initialization:

```cpp
// Player components
m_registry.register_component<PlayerEntity>();
m_registry.register_component<InputState>();

// Enemy / Gameplay
m_registry.register_component<Enemy>();
m_registry.register_component<Boss>();
m_registry.register_component<Items>();
m_registry.register_component<Collision>();
m_registry.register_component<EnemySpawning>();

// Weapon & Projectile
m_registry.register_component<Weapon>();
m_registry.register_component<Projectile>();
```

Additional components can be registered by subsystems or scenes.

---

## Subsystem Management

### `LoadSubsystem(SubsystemType type, const std::string& pluginPath)`

Loads a subsystem plugin:

1. Checks if subsystem is already loaded.
2. Uses `DLLoader<ISubsystem>` to load the plugin.
3. Calls `CreateSubsystem()` factory function from the plugin.
4. Initializes the subsystem.
5. Provides registry access via `SetRegistry()`.
6. Stores the subsystem and adds it to update order.

### `UnloadSubsystem(SubsystemType type)`

Unloads a subsystem:

1. Calls `Shutdown()` on the subsystem.
2. Removes from the subsystem map and update order.
3. DLLoader destructor handles library cleanup.

### `GetSubsystem(SubsystemType type)`

Returns a pointer to the requested subsystem or `nullptr` if not loaded.

---

## Usage Example

```cpp
GameEngine engine;

// Initialize engine
if (!engine.Initialize("My Game", 1280, 720))
    return -1;

// Load subsystems
engine.LoadSubsystem(SubsystemType::RENDERING, "lib/libRendering.so");
engine.LoadSubsystem(SubsystemType::PHYSICS, "lib/libPhysics.so");
engine.LoadSubsystem(SubsystemType::INPUT, "lib/libInput.so");
engine.LoadSubsystem(SubsystemType::AUDIO, "lib/libAudio.so");

// Load scenes
engine.LoadSceneModule("menu", "lib/libMenuScene.so");
engine.LoadSceneModule("game", "lib/libGameScene.so");

// Start with menu scene
engine.ChangeScene("menu");

// Run game loop
engine.Run();

// Cleanup
engine.Shutdown();
```

---

# ECS

## Overview

This Entity–Component–System implementation consists of four core elements:

1. **Entity** – a lightweight numeric ID wrapper.
2. **SparseArray<T>** – optional component storage indexed by entity ID.
3. **Registry** – owns entities, components, and systems.
4. **Zipper / IndexedZipper** – multi-component iterators for system execution.

---

## Entities

```cpp
class Entity {
public:
    explicit Entity(size_t id);
    operator size_t() const;
    size_t id() const;
};
```

### Key Points

* Created exclusively by `Registry::spawn_entity()`.
* Implicit conversion to `size_t` allows direct indexing into component arrays.
* Entity IDs remain stable until deleted.

---

## SparseArray<T>

A compact component container storing elements as:

```cpp
std::vector<std::optional<T>>
```

### Main Operations

| Function | Description |
|----------|-------------|
| `operator[](size_t)` | Access or create the optional at an index. Auto-resizes. |
| `insert_at(pos, value)` | Insert or overwrite a component. |
| `emplace_at(pos, args...)` | Construct component in-place. |
| `erase(pos)` | Remove component at index. |
| `size()` | Current size of the internal vector. |

### Behavior Notes

* Out-of-bounds writes automatically grow the underlying vector.
* Absent components are represented as `std::nullopt`.
* Component lifetime ends immediately when erased.

---

## Registry

The registry coordinates entity management, component storage, and system execution.

### Component Registration

```cpp
auto& positions = registry.register_component<Position>();
```

* Registers a new `SparseArray<T>` stored via `std::any`.
* Allows only one storage array per component type.
* Automatically registers cleanup functions for entity deletion.

### Component Access & Manipulation

| Operation | Description |
|-----------|-------------|
| `add_component(e, value)` | Adds or replaces a component. |
| `emplace_component(e, args...)` | Constructs a component directly. |
| `remove_component<T>(e)` | Removes the component. |
| `get_components<T>()` | Returns the entire component array. |
| `has_component<T>(e)` | Checks if entity has component. |

### Entity Management

```cpp
Entity e = registry.spawn_entity();
registry.kill_entity(e);
registry.clear_all_entities();
```

* Killing an entity triggers removal of all its components.
* Entity IDs increment monotonically.
* `clear_all_entities()` removes all entities and resets state.

### Systems

Systems are registered as templated functions:

```cpp
registry.add_system<A, B>([](Registry& r,
    SparseArray<A>& a, SparseArray<B>& b) {
    // system logic
});
```

Run all systems:

```cpp
registry.run_systems();
```

---

## Zipper & IndexedZipper Iteration

### Zipper

Yields tuples of component references:

```cpp
Zipper zip(positions, velocities);
for (auto [pos, vel] : zip) {
    // Operates only where both components exist
}
```

### IndexedZipper

Yields entity ID plus component references:

```cpp
IndexedZipper iz(posArray, healthArray);
for (auto [id, pos, health] : iz) {
    // id == entity index
}
```

---

# Scene Management

## Overview

The scene system provides a plugin-based architecture for organizing game states (menus, levels, etc.). Scenes are loaded as dynamic libraries and managed by the `SceneManager`.

---

## Scene Base Class

```cpp
class Scene {
protected:
    GameEngine* m_engine;
    SceneManager* m_sceneManager;
    std::string m_name;

public:
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void HandleEvent(SDL_Event& event) {}
    virtual std::unordered_map<uint16_t, Entity> GetPlayers() = 0;
};
```

### Protected Helper Methods

Scenes have access to engine functionality through helper methods:

```cpp
Registry& GetRegistry();
RenderingSubsystem* GetRendering();
AudioSubsystem* GetAudio();
InputSubsystem* GetInput();
NetworkSubsystem* GetNetwork();
UIManager* GetUI();
SceneData& GetSceneData();
void ChangeScene(const std::string& sceneName);
void QuitGame();
```

---

## SceneManager

### Responsibilities

* Loads scene plugins dynamically.
* Manages scene transitions safely.
* Provides persistent data storage via `SceneData`.
* Clears registry between scene transitions.

### Key Methods

```cpp
bool LoadSceneModule(const std::string& name, const std::string& soPath);
void UnloadSceneModule(const std::string& name);
void ChangeScene(const std::string& name);
void Update(float deltaTime);
void HandleEvent(SDL_Event& event);
void ClearAllScenes();
```

### Scene Transitions

Scene transitions happen in `Update()`:

1. If `m_nextScene` is set, begin transition.
2. Call `OnExit()` on current scene.
3. Clear all entities from registry.
4. Set new current scene and call `OnEnter()`.
5. Reset `m_nextScene` to nullptr.

---

## SceneData

Provides type-safe persistent data storage between scenes:

```cpp
template <typename T>
void Set(const std::string& key, const T& value);

template <typename T>
T Get(const std::string& key, const T& defaultValue = T()) const;

bool Has(const std::string& key) const;
void Clear();
void Remove(const std::string& key);
```

### Example

```cpp
// In menu scene
GetSceneData().Set<int>("selectedLevel", 3);

// In game scene
int level = GetSceneData().Get<int>("selectedLevel", 1);
```

---

## Creating a Scene Plugin

```cpp
#include "scene/Scene.hpp"

class MyGameScene : public Scene {
public:
    MyGameScene() { m_name = "game"; }
    
    void OnEnter() override {
        // Initialize entities, systems, etc.
    }
    
    void OnExit() override {
        // Cleanup if needed
    }
    
    void Update(float deltaTime) override {
        // Update game logic
    }
    
    void Render() override {
        // Rendering is typically handled by RenderingSubsystem
    }
    
    std::unordered_map<uint16_t, Entity> GetPlayers() override {
        // Return player entities for networking
    }
};

extern "C" {
    Scene* CreateScene() {
        return new MyGameScene();
    }
}
```

---

# Subsystems

## ISubsystem Interface

All subsystems implement this interface:

```cpp
class ISubsystem {
public:
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void SetRegistry(Registry* registry) = 0;
    virtual void ProcessEvent(SDL_Event event) = 0;
    
    virtual const char* GetName() const = 0;
    virtual SubsystemType GetType() const = 0;
    virtual const char* GetVersion() const = 0;
};
```

### SubsystemType Enum

```cpp
enum class SubsystemType {
    RENDERING,
    PHYSICS,
    AUDIO,
    INPUT,
    RESOURCE,
    MESSAGING,
    NETWORK
};
```

---

## Rendering Subsystem

### Overview

Manages SDL rendering, textures, animations, tilemaps, and UI elements.

### Key Components

* **TextureManager** – Loads and caches SDL textures.
* **AnimationManager** – Manages animation clips.
* **Tilemap** – Multi-layer tilemap rendering with parallax.
* **UIManager** – UI element management and rendering.

### Registered Components

```cpp
Transform      // Position, scale, rotation
Sprite         // Texture key, source rect, pivot, layer
Animation      // Current animation state
Camera         // Camera position and active state
```

### Systems

```cpp
animation_system()              // Updates animation frames
sprite_render_system()          // Renders sprites
layered_sprite_render_system()  // Renders sprites sorted by layer
tilemap_render_system()         // Renders tilemap layers
```

### UI System

The rendering subsystem includes a complete UI system:

#### UIElement (Base Class)

```cpp
class UIElement {
protected:
    SDL_Rect m_rect;
    bool m_visible;
    
public:
    virtual void Render(SDL_Renderer* renderer) = 0;
    virtual void HandleEvent(const SDL_Event& event) = 0;
};
```

#### Available UI Elements

* **UIText** – Rendered text with TTF fonts
* **UIButton** – Interactive button with hover/click states
* **UIImage** – Static or tiled images
* **UITextInput** – Text input field with cursor
* **UISolidColor** – Colored rectangles

#### UIManager

```cpp
class UIManager {
public:
    void AddElement(const std::string& id, std::unique_ptr<UIElement> element);
    void RemoveElement(const std::string& id);
    UIElement* GetElement(const std::string& id);
    void RenderAll(SDL_Renderer* renderer);
    void HandleEvent(const SDL_Event& event);
};
```

---

## Physics Subsystem

### Overview

Provides 2D physics simulation with gravity, velocity, and AABB collision detection/resolution.

### Registered Components

```cpp
RigidBody     // Mass, velocity, acceleration, restitution, isStatic
BoxCollider   // Width, height, offset, isSolid
```

### Systems

```cpp
physics_movement_system()       // Applies gravity and integrates motion
collision_detection_system()    // Detects AABB collisions
collision_resolution_system()   // Resolves collisions with impulses
physics_system()                // Combined system running both
```

### Physics2D Header

Contains physics component definitions and utility functions:

```cpp
bool check_collision(Transform& ta, BoxCollider& ca,
                    Transform& tb, BoxCollider& cb);
                    
void resolve_collision(RigidBody& a, RigidBody& b,
                      Transform& ta, Transform& tb,
                      BoxCollider& ca, BoxCollider& cb);
```

---

## Input Subsystem

### Overview

Handles keyboard and mouse input with configurable key bindings.

### Registered Components

```cpp
InputState  // Stores input state per entity
```

### KeyBindings System

Provides configurable action mapping:

```cpp
class KeyBindings {
public:
    void BindKey(const std::string& action, SDL_Keycode key);
    void UnbindKey(const std::string& action);
    bool IsActionPressed(const std::string& action) const;
    bool IsActionJustPressed(const std::string& action) const;
    bool IsActionJustReleased(const std::string& action) const;
    void Update(const Uint8* keyboardState);
};
```

### Usage Example

```cpp
auto* input = engine.GetSubsystem(SubsystemType::INPUT);
auto* inputSys = dynamic_cast<InputSubsystem*>(input);

inputSys->GetKeyBindings().BindKey("jump", SDLK_SPACE);
inputSys->GetKeyBindings().BindKey("shoot", SDLK_LCTRL);

if (inputSys->GetKeyBindings().IsActionJustPressed("jump")) {
    // Handle jump
}
```

---

## Audio Subsystem

### Overview

Manages audio playback using SDL_mixer with support for music and sound effects.

### Key Features

* Music playback (streaming)
* Sound effect playback (loaded into memory)
* Volume control
* Audio resource management

### API Example

```cpp
auto* audio = dynamic_cast<AudioSubsystem*>(
    engine.GetSubsystem(SubsystemType::AUDIO));

audio->LoadMusic("bgm", "assets/music.mp3");
audio->LoadSound("jump", "assets/jump.wav");

audio->PlayMusic("bgm", -1);  // Loop indefinitely
audio->PlaySound("jump", 0);   // Play once
```

---

## Network Subsystem

### Overview

Provides UDP-based networking with circular buffer for packet handling.

### CircularBuffer

A thread-safe ring buffer for network packets:

```cpp
template<typename T, size_t Size>
class CircularBuffer {
public:
    bool push(const T& item);
    bool pop(T& item);
    bool isEmpty() const;
    bool isFull() const;
    size_t size() const;
};
```

### Network Features

* UDP socket communication
* Packet serialization/deserialization
* Client-server architecture support
* Non-blocking I/O

---

## Resource Subsystem

### Overview

Manages loading and caching of game resources with support for various asset types.

### Responsibilities

* Asset loading from disk
* Resource caching and lifetime management
* Format-specific loaders
* Resource reference counting

---

## Messaging Subsystem

### Overview

Provides inter-system communication via an event/message system.

### Key Features

* Type-safe message passing
* Subscribe/publish pattern
* Decoupled system communication
* Message queuing

### Usage Pattern

```cpp
// Subscribe to messages
messaging->Subscribe<PlayerDeathEvent>(
    [](const PlayerDeathEvent& evt) {
        // Handle player death
    });

// Publish messages
messaging->Publish(PlayerDeathEvent{playerId});
```

---

## Best Practices

### Engine Usage

* Load all required subsystems during initialization.
* Load subsystems in dependency order (e.g., INPUT before others).
* Use `SceneData` for persistent data between scenes.
* Clear registry between scene transitions for clean state.

### Scene Development

* Initialize all entities in `OnEnter()`.
* Clean up in `OnExit()` only if necessary (registry auto-clears).
* Use `GetSceneData()` for cross-scene communication.
* Keep scene logic modular and self-contained.

### Component Design

* Keep components as pure data structures.
* Avoid logic in components.
* Use small, focused components.
* Leverage component composition.

### System Design

* Systems should be stateless.
* Process only entities with required components.
* Use `IndexedZipper` when you need entity IDs.
* Keep systems focused on single responsibilities.

---
