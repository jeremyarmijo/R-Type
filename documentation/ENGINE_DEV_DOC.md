# Game Engine Developer Documentation

## Overview

- [**Game Engine**](#game-engine)
- [**ECS**](#ECS)
- [**Rendering Systems**](#rendering-systems)
- [**Texture Manager**](#texture-manager)
- [**Animation Manager**](#animation-manager)
- [**Physics Systems**](#physics-systems)
- [**Tile Map Manager**](#tilemap)
- **Audio Manager** TODO
- **Network Manager** TODO

---

# Game Engine

The `GameEngine` class acts as the core runtime manager responsible for initializing subsystems, handling game loop execution, updating simulation state, and rendering all visual output. It encapsulates SDL window management, ECS registry orchestration, asset managers, networking, physics, and tilemap rendering.

---

## Responsibilities

### Core Engine Lifecycle

* Create and manage the SDL window and renderer.
* Initialize and shut down SDL, SDL_image, and engine subsystems.
* Maintain the main game loop (`Run()`).
* Process events and input.
* Update the simulation based on elapsed time.
* Render all visual content in correct draw order.

### ECS (Entity–Component–System)

* Stores an internal `Registry` instance.
* Registers all engine-level components and systems.
* Provides helper functions to create common entity archetypes:

  * Sprites
  * Animated sprites
  * Physics-enabled objects

### Rendering and Assets

* Owns the `TextureManager` for loading and caching textures.
* Owns the `AnimationManager` for managing animation definitions.
* Renders sprites and tile layers in depth-ordered layers.
* Maintains camera position and applies it to all rendering.

### Physics

* Stores global gravity vector.
* Runs movement and collision systems each frame.

### Networking

* Owns the `NetworkManager`.
* Provides simple connection API.

### Tilemap

* Owns a `Tilemap` instance used for world rendering and collision queries.
* Renders tilemap layers before sprites.

---

## Main Loop Overview

### `Run()`

Drives the entire game lifecycle:

1. Compute frame delta-time.
2. Process input (`HandleEvents()`).
3. Update ECS, physics, and camera (`Update()`).
4. Draw tilemap, sprites, and debug overlays (`Render()`).

### `Update(deltaTime)`

* Runs all registered ECS systems in correct dependency order.
* Applies physics, animation updates, and collision handling.
* Adjusts camera position to follow a chosen entity.

### `Render()`

* Clears screen.
* Draws tilemap layers (`tilemap_render_system`).
* Draws all sprite layers (`layered_sprite_render_system`).
* Displays optional debug information.
* Presents final frame.

---

## Subsystem Registration

### Components

Automatically registered by `RegisterComponents()` e.g. :

* `Transform`
* `RigidBody`
* `BoxCollider`
* `Sprite`
* `Animation`
* `Camera`

### Systems

Added through `RegisterSystems()` in execution order e.g. :

1. Animation update
2. Physics movement
3. Collision detection

Systems are executed each frame through `Registry::run_systems()`.

---

## Camera

A simple 2D camera controlled internally by the engine:

---

## Asset Managers

### `TextureManager`

Used for:

* Loading textures on demand.
* Caching and retrieving SDL textures via string keys.

### `AnimationManager`

Responsible for:

* Storing animation clips.
* Updating animation playback in the animation system.

---

## Usage Example

```cpp
GameEngine engine;

if (!engine.Initialize("My Game", 1280, 720))
    return -1;

engine.GetTilemap().AddLayer("background", 0.5f);
engine.GetTilemap().AddLayer("world", 1.0f);

engine.CreateSprite("player.png", {100, 100});
engine.CreatePhysicsObject("crate.png", {200, 100}, {32, 32}, false);

engine.Run();
engine.Shutdown();
```

---

# ECS

## **Overview**

This Entity–Component–System implementation is composed of four core elements:

1. **Entity** — a lightweight numeric ID wrapper.
2. **SparseArray<T>** — optional component storage indexed by entity ID.
3. **Registry** — owns entities, components, and systems.
4. **Zipper / IndexedZipper** — multi-component iterators for system execution.

---

# **Entities**

```cpp
class Entity {
public:
    explicit Entity(size_t id);
    operator size_t() const;
    size_t id() const;
};
```

### **Key Points**

* Created exclusively by `Registry::spawn_entity()`.
* Implicit conversion to `size_t` allows direct indexing into component arrays.
* Entity IDs remain stable until deleted.

---

# **SparseArray<T>**

A compact component container storing elements as:

```cpp
std::vector<std::optional<T>>
```

### **Main Operations**

| Function                   | Description                                              |
| -------------------------- | -------------------------------------------------------- |
| `operator[](size_t)`       | Access or create the optional at an index. Auto-resizes. |
| `insert_at(pos, value)`    | Insert or overwrite a component.                         |
| `emplace_at(pos, args...)` | Construct component in-place.                            |
| `erase(pos)`               | Remove component at index.                               |
| `size()`                   | Current size of the internal vector.                     |

### **Behavior Notes**

* Out-of-bounds writes automatically grow the underlying vector.
* Absent components are represented as `std::nullopt`.
* Component lifetime ends immediately when erased.

---

# **Registry**

The registry coordinates entity management, component storage, and system execution.

## **Component Registration**

```cpp
auto& positions = registry.register_component<Position>();
```

* Registers a new `SparseArray<T>` stored via `std::any`.
* Allows only one storage array per component type.

## **Component Access & Manipulation**

| Operation                       | Description                         |
| ------------------------------- | ----------------------------------- |
| `add_component(e, value)`       | Adds or replaces a component.       |
| `emplace_component(e, args...)` | Constructs a component directly.    |
| `remove_component(e)`           | Removes the component.              |
| `get_components<T>()`           | Returns the entire component array. |

## **Entity Management**

```cpp
Entity e = registry.spawn_entity();
registry.kill_entity(e);
```

* Killing an entity triggers the removal of all registered component types.
* Entity IDs are reused only in the sense that they increment; deleted IDs are not reclaimed.

## **Systems**

Systems are registered as templated functions:

```cpp
registry.add_system<A, B>([](Registry& r,
    SparseArray<A>& a, SparseArray<B>& b) {
    // system logic
});
```

A system receives:

* the registry itself,
* references to the component arrays for the listed types.

Run all systems:

```cpp
registry.run_systems();
```

---

# **Zipper & IndexedZipper Iteration**

These utilities allow synchronous iteration over multiple component arrays.

---

## **Zipper**

Yields tuples of component references:

```cpp
std::tuple<A&, B&, C&>
```

### **Example**

```cpp
Zipper zip(positions, velocities);

for (auto [pos, vel] : zip) {
    // Operates only where both components exist
}
```

### **Characteristics**

* Iterates up to the maximum size of all arrays.
* Skips indices where any container lacks a value.

---

## **IndexedZipper**

Yields:

```cpp
std::tuple<size_t, A&, B&, ...>
```

The first element is the entity ID.

### **Example**

```cpp
IndexedZipper iz(posArray, healthArray);

for (auto [id, pos, health] : iz) {
    // id == entity index
}
```

---

# **Usage Pattern**

## **1. Define Components**

```cpp
struct Position { float x, y; };
struct Velocity { float vx, vy; };
```

## **2. Register Components**

```cpp
Registry r;
r.register_component<Position>();
r.register_component<Velocity>();
```

## **3. Create Entities**

```cpp
Entity e = r.spawn_entity();
r.emplace_component<Position>(e, 0.f, 0.f);
r.emplace_component<Velocity>(e, 1.f, 0.5f);
```

## **4. Add Systems**

```cpp
r.add_system<Transform, RigidBody>(
    [this](Registry& reg, SparseArray<Transform>& transforms, SparseArray<RigidBody>& rigidbodies) {
        physics_movement_system(reg, transforms, rigidbodies, m_deltaTime, m_gravity);
    }
);
```

## **5. Run Systems**

```cpp
r.run_systems();
```

---

# Rendering Systems

## Overview

1. **Animation System** – Updates frame progression for animations.
2. **Sprite Render System** – Renders sprites using positions, pivots, and scaling.
3. **Layered Sprite Render System** – Renders sprites sorted by layer order.
4. **Tilemap Render System** - Renders Tilemap sorted by layer order.
5. **Camera Getter** – Returns active camera position.

All systems operate on an ECS `Registry` and use `SparseArray` component storage with Zipper iterators.

---

# Animation System

## `animation_system(...)`

### Purpose

Advances the animation frames of all entities that have both `Animation` and `Sprite` components.

### Parameters

| Name               | Type                      | Description                        |
| ------------------ | ------------------------- | ---------------------------------- |
| `registry`         | `Registry&`               | ECS registry handle.               |
| `animations`       | `SparseArray<Animation>&` | Animation components for entities. |
| `sprites`          | `SparseArray<Sprite>&`    | Sprite components for entities.    |
| `animationManager` | `AnimationManager*`       | Provides animation clips.          |
| `deltaTime`        | `float`                   | Time elapsed since the last frame. |

### Behavior

* Skips entities with no active animation.
* Retrieves the animation clip by name.
* Accumulates `currentTime` and checks if the next frame should be displayed.
* Loops or stops animation depending on the clip’s settings.
* Updates the sprite’s `sourceRect` each frame.

### Example Use

```cpp
animation_system(registry, animations, sprites, &animationManager, deltaTime);
```

---

# Sprite Render System

## `sprite_render_system(...)`

### Purpose

Renders sprites using world transforms, respecting camera position, scaling, rotation, and pivot.

### Parameters

| Name             | Type                      | Description                      |
| ---------------- | ------------------------- | -------------------------------- |
| `registry`       | `Registry&`               | ECS registry handle.             |
| `transforms`     | `SparseArray<Transform>&` | Position, scale, rotation.       |
| `sprites`        | `SparseArray<Sprite>&`    | Texture key, pivot, source rect. |
| `textureManager` | `TextureManager*`         | Fetches textures.                |
| `renderer`       | `SDL_Renderer*`           | SDL renderer.                    |
| `cameraPosition` | `Vector2`                 | Offset applied to all sprites.   |

### Behavior

* Retrieves the sprite’s texture.
* If the sprite has no sourceRect, gets the full texture size.
* Computes `destRect` from transform + pivot + camera.
* Renders using `SDL_RenderCopyEx`.

### Example

```cpp
sprite_render_system(registry, transforms, sprites, &textureManager, renderer, cameraPos);
```

---

# Layered Sprite Render System

## `layered_sprite_render_system(...)`

### Purpose

Renders all sprites **sorted by layer value**, allowing foreground/background effects.

### Parameters

Same as the basic sprite renderer, but the system:

* Collects all entities into a list.
* Sorts them by `sprite.layer`.
* Renders in sorted order.

### Notes

* Requires that `Sprite` includes a `layer` integer.
* Higher or lower values can meaningfully define depth based on your convention.

### Example

```cpp
layered_sprite_render_system(registry, transforms, sprites, &textureManager, renderer, cameraPos);
```

---

## `tilemap_render_system(...)`

### Purpose

Renders a tilemap with support for multiple layers, parallax scrolling, and frustum culling to improve performance.

### Parameters

| Name             | Type              | Description                                          |
| ---------------- | ----------------- | ---------------------------------------------------- |
| `tilemap`        | `Tilemap*`        | Tilemap instance containing layers and tileset info. |
| `textureManager` | `TextureManager*` | Used to fetch the tileset texture.                   |
| `renderer`       | `SDL_Renderer*`   | SDL renderer used to draw tile quads.                |
| `cameraPosition` | `Vector2`         | World-space camera offset.                           |
| `windowWidth`    | `int`             | Width of the render viewport.                        |
| `windowHeight`   | `int`             | Height of the render viewport.                       |

### Behavior

* Retrieves the tileset texture from the texture manager.
* Iterates over all layers in the tilemap.
* Applies each layer’s individual parallax factor to the camera position.
* Computes the tile range visible within the screen bounds (frustum culling).
* Iterates only through visible tiles and renders each one using its source rectangle.
* Skips empty tiles (`textureID < 0`).

### Example

```cpp
tilemap_render_system(tilemap, &textureManager, renderer, cameraPos, winW, winH);
```

---

# Camera System

## `get_camera_position(...)`

### Purpose

Finds the first active camera and returns its position.

### Parameters

| Name         | Type                      | Description |
| ------------ | ------------------------- | ----------- |
| `registry`   | `Registry&`               |             |
| `cameras`    | `SparseArray<Camera>&`    |             |
| `transforms` | `SparseArray<Transform>&` |             |

### Behavior

* Uses `IndexedZipper` to iterate aligned cameras and transforms.
* If a camera has `isActive = true`, returns its stored position.
* If none are active, returns `(0, 0)`.

### Example

```cpp
Vector2 camPos = get_camera_position(registry, cameras, transforms);
```

---

# Header Summary

These functions are declared in the module header:

```cpp
void animation_system(...);
void sprite_render_system(...);
void layered_sprite_render_system(...);
Vector2 get_camera_position(...);
```

---

# Texture Manager

## Overview

`TextureManager` is a lightweight utility class that loads, stores, and retrieves SDL textures.
It maintains a texture cache using string keys, preventing duplicate loads and simplifying texture access throughout your game engine.

The class depends on:

* **SDL2**
* **SDL2_image**

---

# **Class Summary**

```cpp
class TextureManager {
private:
    std::unordered_map<std::string, SDL_Texture*> m_textures;
    SDL_Renderer* m_renderer;

public:
    TextureManager(SDL_Renderer* renderer);
    ~TextureManager();

    bool LoadTexture(const std::string& key, const std::string& filepath);
    SDL_Texture* GetTexture(const std::string& key);
    void GetTextureSize(const std::string& key, int& width, int& height);
};
```

---

# **Responsibilities**

### Load image files and convert them to SDL textures

### Store textures in a string-indexed cache

### Provide access to textures on demand

### Automatically release texture memory at destruction

---

# **Constructor & Destructor**

## **`TextureManager(SDL_Renderer* renderer)`**

Initializes the manager and stores a pointer to the renderer used for texture creation.

## **`~TextureManager()`**

Destroys all cached textures using `SDL_DestroyTexture()`.
This prevents memory leaks if textures are not manually freed.

---

# **Public Methods**

## **`bool LoadTexture(const std::string& key, const std::string& filepath)`**

Loads an image file using `IMG_Load()`, converts it into an `SDL_Texture`, and stores it in the internal cache.

### **Parameters**

* `key` — unique identifier for the texture
* `filepath` — path to the image file to load

### **Returns**

* `true` if the texture was successfully loaded
* `false` if loading or texture creation failed

### **Notes**

* If a texture with the same key already exists, it is replaced.
* Supports any format handled by **SDL_image** (PNG, JPG, BMP, etc.).

---

## **`SDL_Texture* GetTexture(const std::string& key)`**

Retrieves the texture associated with a given key.

### **Returns**

* Pointer to `SDL_Texture` if the key exists
* `nullptr` if not found

### **Usage**

```cpp
SDL_Texture* tex = textures.GetTexture("player");
if (tex) {
    SDL_RenderCopy(renderer, tex, nullptr, &rect);
}
```

---

## **`void GetTextureSize(const std::string& key, int& width, int& height)`**

Queries the width and height of a loaded texture.

### **Parameters**

* `width` — output variable modified by the function
* `height` — output variable modified by the function

### **Behavior**

If the texture exists, the method calls:

```cpp
SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
```

If not, the width and height remain unchanged.

---

# **Internal Behavior**

### **Texture Storage**

Textures are stored in:

```cpp
std::unordered_map<std::string, SDL_Texture*>
```

This allows:

* fast O(1) lookups
* simple string-based keys for asset management

### **Memory Management**

All textures are destroyed safely in the destructor:

```cpp
for (auto& pair : m_textures) {
    SDL_DestroyTexture(pair.second);
}
```

---

# **Usage Example**

```cpp
TextureManager textures(renderer);

// Load
if (!textures.LoadTexture("player", "assets/player.png")) {
    // handle error
}

// Retrieve
SDL_Texture* playerTex = textures.GetTexture("player");

// Query size
int w, h;
textures.GetTextureSize("player", w, h);

// Render
SDL_Rect dst = { 100, 100, w, h };
SDL_RenderCopy(renderer, playerTex, nullptr, &dst);
```

---

# **Best Practices**

* Load all textures *once* at game initialization.
* Use keys naming conventions: `"enemy_walk"`, `"ui_button"`, etc.
* Avoid loading textures inside your game loop (expensive I/O).
* Ensure `IMG_Init()` is called before using this class.

---

# Animation Manager

## Overview

This module provides a simple animation system built around **frames**, **animation clips**, and an **animation manager**.
It allows you to define animations by grouping frames with specified durations and whether they should loop.

---

## `AnimationFrame`

### Description

Represents a single frame of animation.

### Fields

| Field        | Type       | Description                                          |
| ------------ | ---------- | ---------------------------------------------------- |
| `sourceRect` | `SDL_Rect` | Region of the texture atlas representing this frame. |
| `duration`   | `float`    | Time (in seconds) the frame remains active.          |

### Example

```cpp
AnimationFrame frame{
    SDL_Rect{0, 0, 32, 32},
    0.1f
};
```

---

## `AnimationClip`

### Description

A complete animation composed of multiple `AnimationFrame` entries.

### Fields

| Field    | Type                          | Description                          |
| -------- | ----------------------------- | ------------------------------------ |
| `frames` | `std::vector<AnimationFrame>` | Ordered list of animation frames.    |
| `loop`   | `bool`                        | True if the animation should repeat. |

### Constructor

```cpp
AnimationClip(bool shouldLoop = true);
```

---

## `AnimationManager`

### Description

Stores and retrieves multiple named `AnimationClip` instances.
Acts as the central registry for all animations.

### Private Members

| Name           | Type                                             | Description                |
| -------------- | ------------------------------------------------ | -------------------------- |
| `m_animations` | `std::unordered_map<std::string, AnimationClip>` | Stores animations by name. |

---

## Public Methods

### `void CreateAnimation(const std::string& name, const std::vector<AnimationFrame>& frames, bool loop = true);`

#### Purpose

Creates and registers a new animation clip.

#### Parameters

| Name     | Description                                        |
| -------- | -------------------------------------------------- |
| `name`   | Unique string identifier for the animation.        |
| `frames` | Ordered list of `AnimationFrame` objects.          |
| `loop`   | Whether the animation should loop (default: true). |

#### Example

```cpp
std::vector<AnimationFrame> walkFrames = {
    { SDL_Rect{0, 0, 32, 32}, 0.1f },
    { SDL_Rect{32, 0, 32, 32}, 0.1f },
    { SDL_Rect{64, 0, 32, 32}, 0.1f }
};

animationManager.CreateAnimation("walk", walkFrames, true);
```

---

### `const AnimationClip* GetAnimation(const std::string& name) const;`

#### Purpose

Fetches a previously created animation clip.

#### Returns

Pointer to the `AnimationClip`, or `nullptr` if the name does not exist.

#### Example

```cpp
const AnimationClip* walk = animationManager.GetAnimation("walk");
if (walk) {
    // Use the animation...
}
```

---

## Notes & Best Practices

* Keep animation names unique to avoid accidental replacement.
* Ensure all frames in a clip reference valid areas in your sprite sheet.
* Use short frame durations for smooth animation; longer durations for pauses or cinematic effects.
* This system is compatible with any sprite renderer that uses `SDL_Rect` source regions.

---

Below is a clean **Markdown documentation section for your Physics Systems**, written in the same style and structure as the Render Systems documentation you already have.

---

# Physics Systems

## Overview

This module provides core 2D physics behavior for your ECS:

1. **Movement System** — Applies gravity, acceleration, and velocity to transforms.
2. **Collision Detection System** — Detects AABB collisions between entities with colliders.
3. **Collision Resolution System** — Adjusts velocities after collisions.
4. **Physics System (Combined)** — Runs movement + collision in the correct order.
5. **Utility Functions** — Collision checks and resolution functions.

---

# Movement System

## `physics_movement_system(...)`

### Purpose

Updates entity positions by applying acceleration, gravity, and velocity.

### Parameters

| Name          | Type                      | Description                           |
| ------------- | ------------------------- | ------------------------------------- |
| `registry`    | `Registry&`               | ECS registry handle.                  |
| `transforms`  | `SparseArray<Transform>&` | Transform components to update.       |
| `rigidbodies` | `SparseArray<RigidBody>&` | Physics data (mass, velocity, etc.).  |
| `deltaTime`   | `float`                   | Time elapsed since last update.       |
| `gravity`     | `Vector2`                 | Gravity applied to non-static bodies. |

### Behavior

* Skips static rigidbodies.
* Adds gravity to acceleration.
* Updates velocity and position using integration.
* Clears acceleration after each step.

---

# Collision Detection

## `collision_detection_system(...)`

### Purpose

Checks all eligible entities for **AABB collisions** and dispatches them to the resolver.

### Parameters

| Name          | Type                        | Description |
| ------------- | --------------------------- | ----------- |
| `registry`    | `Registry&`                 |             |
| `transforms`  | `SparseArray<Transform>&`   |             |
| `colliders`   | `SparseArray<BoxCollider>&` |             |
| `rigidbodies` | `SparseArray<RigidBody>&`   |             |

### Behavior

* Collects all entities that have both transform + collider.
* Performs pairwise collision tests (naïve O(n²) broadphase).
* Uses `check_collision` to test bounding boxes.
* Calls `resolve_collision` for any pair where both entities have rigidbodies.

---

# Collision Check

## `check_collision(...)`

### Purpose

Tests AABB vs. AABB intersection.

### Parameters

| Name         | Type          |
| ------------ | ------------- |
| `transformA` | `Transform`   |
| `colliderA`  | `BoxCollider` |
| `transformB` | `Transform`   |
| `colliderB`  | `BoxCollider` |

### Behavior

* Converts colliders into world-space bounds.
* Tests for overlap along X and Y axes.
* Returns true if both axes overlap.

---

# Collision Resolution

## `resolve_collision(...)`

### Purpose

Adjusts velocities after a collision based on mass, restitution (bounciness), and motion.

### Logic

* Computes relative velocity.
* Applies restitution coefficient (min of both bodies).
* Handles dynamic–dynamic and static–dynamic cases.
* Dampens post-collision velocity slightly to avoid infinite bouncing.

---

# Combined Physics System

## `physics_system(...)`

### Purpose

Runs both movement and collision systems in the correct order.

### Order of Operations

1. **Movement integration** (gravity, acceleration, velocity).
2. **Collision detection + resolution**.

### Parameters

| Name        | Type        |
| ----------- | ----------- |
| `registry`  | `Registry&` |
| `deltaTime` | `float`     |
| `gravity`   | `Vector2`   |

### Example

```cpp
physics_system(registry, deltaTime, {0, 9.81f});
```

---

## Tilemap

### `Tile`

Represents a single tile within a layer.

---

## `TilemapLayer`

A single layer of tiles. Layers can scroll at different speeds to create parallax effects.

### Key Features

* Stores `width × height` tiles.
* Supports parallax scrolling (`m_parallaxSpeed`).
* Allows setting tiles and querying tile data.

---

## `Tilemap`

Manages multiple layers, a tileset, and tile metadata such as tile size and world dimensions.

### Responsibilities

* Stores and retrieves layers.
* Converts tile IDs to source rectangles in the tileset.
* Performs collision checks based on tile solidity.
* Exposes world-space metrics (pixel dimensions).

---
