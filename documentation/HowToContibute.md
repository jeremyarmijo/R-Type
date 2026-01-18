# GamePlay Guide – R-TYPE

## Table of Contents

1. [Definition](#definition)  
2. [ECS Architecture](#ecs-architecture)  
3. [Key Features](#key-features)  
4. [Files Organisation](#files-organisation)  
5. [Creating Entities](#creating-entities)  
6. [Components](#components)  
7. [Systems](#systems)  
8. [ServerGame](#servergame)  
9. [Scripting](#scripting)  
10. [Author](#author)

---

## Definition

GamePlay in **R-TYPE** refers to all core mechanics that define how the game behaves and feels during a match.  
It includes player movement, enemy spawning, boss behavior, projectile management, collisions, and game rules.

All gameplay logic is implemented using a **custom Entity-Component-System (ECS) architecture**.

---

## ECS Architecture

The ECS architecture is composed of three main parts:

- **Entities** – Simple identifiers representing game objects (Player, Enemy, Boss, Projectile, etc.)
- **Components** – Data-only structures attached to entities (Transform, Enemy, PlayerEntity, Weapon, etc.)
- **Systems** – Logic that operates on entities owning a specific set of components

### Component Registration Example

```cpp
GetRegistry().register_component<PlayerEntity>();
GetRegistry().register_component<InputState>();
GetRegistry().register_component<Enemy>();
GetRegistry().register_component<Boss>();
GetRegistry().register_component<Projectile>();
```

---

## Key Features

- **Custom ECS Architecture** – Clear separation between data (components) and logic (systems)
- **Multiplayer Client** – Authoritative server with TCP/UDP synchronization
- **Player Movement** – Keyboard-based input via `InputState` component
- **Enemy AI & Wave System** – Multiple enemy types with progressive spawning
- **Boss System** – Multi-phase boss fights with distinct behaviors
- **Projectile System** – Player and enemy projectiles with damage handling
- **Collision & Damage** – AABB collisions with entity destruction
- **Extensible Gameplay** – Easy to add new enemies, weapons, or levels

---

## Files Organisation

```
Shared/
├── components/     → Data definitions for entities
└── systems/        → Game logic implementation
```

**To extend the game:**
- Add components in `Shared/components/` (what entities have)
- Add systems in `Shared/systems/` (what entities do)
- Use helper functions in `Shared/Helper/EntityHelper.hpp`

---

## Creating Entities

Entities are created using **helper functions** in `Shared/Helper/EntityHelper.hpp`:

### Player
```cpp
Entity createPlayer(Registry& registry, const Vector2& startPos, int playerId);
```
**Components:** Transform, RigidBody, InputState, Weapon, BoxCollider, PlayerEntity

### Enemies
```cpp
Entity createEnemy(Registry& registry, EnemyType type, const Vector2& startPos);
```
**Types:** Basic, Zigzag, Chase, mini_Green, Spinner  
**Components:** Transform, RigidBody, BoxCollider, Enemy

### Boss
```cpp
Entity createBoss(Registry& registry, BossType type, Vector2 pos, BossPhase phase, int hp);
```
**Types:** BigShip, Snake, FinalBoss  
**Components:** Transform, RigidBody, BoxCollider, Boss

### Projectiles
```cpp
Entity spawn_projectile(Registry& registry, Entity owner, Vector2 pos, Vector2 dir, float damage);
```
**Components:** Transform, RigidBody, BoxCollider, Projectile

### Force
The **Force** is a detachable pod that follows the player or can be launched forward.  
**States:** AttachedFront, AttachedBack, Detached  
**Features:** Contact damage, blocks enemy projectiles

---

## Components

**Path:** `Shared/components/`

Components are pure data structures with no logic.

### Core Components

```cpp
struct Transform {
    Vector2 position;
};

struct RigidBody {
    Vector2 velocity;
    float mass;
    float friction;
    bool isStatic;
};

struct BoxCollider {
    float width;
    float height;
};
```

### Gameplay Components

```cpp
struct PlayerEntity {
    int playerId;
    float speed;
    int health;
    int maxHealth;
    int score;
    PlayerState state;
    float invincibilityTimer;
};

struct Enemy {
    EnemyType type;
    float health;
};

struct Boss {
    BossType type;
    BossPhase currentPhase;
    float health;
    float maxHealth;
};

struct Projectile {
    float damage;
    float speed;
    float lifetime;
    float currentLife;
    ProjectileOwner owner;
};

struct Weapon {
    float fireRate;
    float cooldown;
    float damage;
};

struct InputState {
    bool moveLeft, moveRight, moveUp, moveDown;
    bool shoot, detachForce;
};

struct LevelComponent {
    std::vector<Wave> waves;
    int currentWave;
    float waveDelayTimer;
    bool finishedLevel;
};

struct Force {
    Entity ownerPlayer;
    EForceState state;
    float contactDamage;
    bool blocksProjectiles;
};
```

### Adding a New Component

1. Create `MyComponent.hpp` in `Shared/components/`
2. Register: `GetRegistry().register_component<MyComponent>()`
3. Add to entities: `registry.add_component<MyComponent>(entity, data)`
4. Create system in `Shared/systems/` to use it

---

## Systems

**Path:** `Shared/systems/`

Systems contain logic that operates on components.

### Movement System
**Path:** `Shared/system/Movement`

Manages all entity movement:
- **Player:** Reads `InputState`, updates `RigidBody.velocity`
- **Enemies:** AI patterns (Zigzag, Chase, Spinner)
- **Bosses:** Multi-phase patterns, spawns minions
- **Projectiles:** Linear movement, lifetime tracking

**Required components:** Transform, RigidBody, type-specific component

```cpp
// Example: Player movement
for (auto&& [state, rb, player] : Zipper(states, rigidbodies, players)) {
    rb.velocity = {0.f, 0.f};
    if (state.moveLeft) rb.velocity.x = -player.speed;
    if (state.moveRight) rb.velocity.x = player.speed;
    // ...
}
```

---

### Collision System
**Path:** `Shared/system/Collision`

Handles all entity interactions using AABB collision detection.

**Required components:** Transform, BoxCollider

**Behavior:**
- Detects collisions between entities
- Applies damage based on entity types
- Updates player score on kills
- Handles invincibility timers

**Adding collision for new entity:**
1. Add `Transform` + `BoxCollider` components
2. Update collision category in `Collision.cpp`
3. Add damage logic in `Projectile_Collision_system()`

---

### Projectile System
**Path:** `Shared/system/Projectile`

Manages projectile lifecycle and collision.

**Key Functions:**
- `spawn_projectile()` – Creates projectile entity
- `apply_projectile_damage()` – Applies damage to targets
- `projectile_collision_system()` – Detects hits and destroys projectiles

**Required components:** Transform, RigidBody, BoxCollider, Projectile

---

### Level System
**Path:** `Shared/system/LevelSystem`

Manages wave-based progression.

**Components:**
```cpp
struct Wave {
    std::vector<EnemyType> enemyTypes;
    std::vector<int> enemiesPerType;
    std::vector<Vector2> spawnPositions;
    bool isBossWave;
    std::optional<BossType> bossType;
    int bossHP;
};
```

**Behavior:**
- Waits for all enemies/bosses to die
- 3-second delay between waves
- Difficulty scaling per level (speed/HP multipliers)
- Special boss spawning (Snake segments, Battleship turrets)

**Adding new levels:** Edit `createLevels()` in `LevelSystem.cpp`

```cpp
levels.emplace_back(std::vector<Wave>{
    Wave{{EnemyType::Basic}, {5}, {{700, 100}}, false, std::nullopt, 0},
    Wave{{}, {0}, {{700, 250}}, true, BossType::BigShip, 1000}
});
```

---

### Force System
**Path:** `Shared/system/ForceCtrl`

Manages the detachable Force Pod.

**Key Functions:**
- `force_control_system()` – Handles detach/recall input
- `force_collision_system()` – Applies contact damage, blocks projectiles

**States:**
- **AttachedFront/Back:** Follows player
- **Detached:** Moves forward independently

**Combat:**
- Contact damage to enemies/bosses
- Blocks enemy projectiles
- Awards score to player on kills

---

## ServerGame

**Path:** `Server/ServerGame`

Manages server-side game logic for multiplayer.

**Key Responsibilities:**
- Player authentication and spawning
- Network message handling (TCP/UDP)
- Game loop (~60 FPS fixed timestep)
- World state broadcasting to clients
- Systems integration (movement, collision, projectiles, waves)

### How to Contribute

1. **Add Player Actions** – Extend `PLAYER_INPUT` struct, update `ReceivePlayerInputs()`
2. **Add Systems** – Create custom ECS systems, call in `UpdateGameState()`
3. **Network Messages** – Extend `Event`/`Action` structures, implement encode/decode
4. **Gameplay Mechanics** – Modify enemy/boss behaviors, ensure updates sent to clients
5. **Debug** – Use `std::cout` for logging, add debug messages for new systems

> **Tip:** The server maintains authoritative game state. Clients only send inputs.

---

## Scripting

**Path:** `Shared/scripting/` (planned feature)

> **Status:** Lua scripting is **planned but not yet implemented**.

### Planned Features
- Define enemy waves and boss behaviors via `.lua` files
- Hot-reload gameplay changes without recompiling
- Configure levels, weapons, and attack patterns externally

### Example
```lua
wave = {
    id = 1,
    enemies = {
        {type = "Basic", count = 5, spawn_interval = 1.0},
        {type = "Zigzag", count = 3, spawn_interval = 2.0}
    }
}
```

### Current Workaround
- Edit `LevelSystem.cpp` → `createLevels()` for waves
- Modify C++ enums/structs for behaviors
- Recompile after changes

### For Contributors
To implement Lua support:
1. Add Lua 5.4+ library dependency
2. Create C++/Lua bindings in `Shared/scripting/`
3. Expose ECS components (read-only recommended)
4. Implement hot-reload system

**Resources:** [Lua Manual](https://www.lua.org/manual/5.4/) | [C API Tutorial](https://www.lua.org/pil/24.html)

---

## Author

dalia.guiz@epitech.eu