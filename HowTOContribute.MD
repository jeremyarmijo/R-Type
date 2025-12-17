# GamePlay Guide – R-TYPE
# Table of Contents

1. [Definition](#definition)  
2. [ECS Architecture Overview](#ecs-architecture-overview)  
3. [Key Gameplay Features](#key-gameplay-features)  
4. [Creating Entities](#creating-entities)  
   - [Player](#player)  
   - [Enemies](#enemies)  
   - [Boss](#boss)  
   - [Projectiles](#projectiles)  
5. [Movement System](#movement-system)  
6. [Collision and Damage System](#collision-system)  
7. [Projectile System](#projectile-system)    
8. [ServerGame](#servergame)

---

## Definition

GamePlay in **R-TYPE** refers to all core mechanics that define how the game behaves and feels during a match.  
It includes player movement, enemy spawning, boss behavior, projectile management, collisions, and game rules.

All gameplay logic is implemented using a **custom Entity-Component-System (ECS) architecture**, ensuring:
- High modularity  
- Clear separation of responsibilities  
- Easy extensibility  

---

## ECS Architecture Overview

The ECS architecture is composed of three main parts:

- **Entities**  
  Simple identifiers representing game objects (Player, Enemy, Boss, Projectile, etc.).

- **Components**  
  Data-only structures attached to entities (Transform, Enemy, PlayerEntity, Weapon, etc.).

- **Systems**  
  Logic that operates on entities owning a specific set of components.

### Component Registration Example

```cpp
registry.register_component<Transform>();
registry.register_component<RigidBody>();
registry.register_component<BoxCollider>();
registry.register_component<PlayerEntity>();
registry.register_component<InputState>();
registry.register_component<Enemy>();
registry.register_component<Boss>();
registry.register_component<Weapon>();
registry.register_component<Projectile>();

```

## Key Features

- **Custom ECS Architecture**
  - Fully custom Entity-Component-System (ECS) design
  - Clear separation between data (components) and logic (systems)

- **Multiplayer Client**
  - Authoritative server managing the entire game state
  - Real-time synchronization using TCP and UDP
  - Supports multiple players in the same game session

- **Player Movement**
  - Keyboard-based movement 
  - Input handled through an `InputState` component
  - Physics-based movement using `RigidBody` and `Transform`

- **Enemy AI & Wave System**
  - Multiple enemy types (Basic, Zigzag, Chase)
  - Progressive wave-based spawning system
  - Difficulty scaling over time

- **Boss System**
  - Multiple boss types with distinct behaviors
  - Multi-phase boss fights
  - High health

- **Projectile & Weapon System**
  - Player and enemy projectiles
  - Weapon cooldown and reload systems
  - Projectile lifetime and damage handling

- **Collision & Damage System**
  - Axis-aligned bounding box (AABB) collisions
  - Damage applied based on entity type (Player, Enemy, Boss)
  - Entity destruction handled by the ECS registry


- **Modular & Extensible Gameplay**
  - New enemies, weapons, or mechanics can be added easily
  - Systems are independent and reusable
  - Designed for scalability and maintainability


## Creating Entities

All game objects in R-TYPE (players, enemies, bosses, projectiles) are created using **helper functions**.  
These helpers encapsulate entity creation and component assignment, ensuring consistency and reducing boilerplate.

Each entity is:
- Spawned via the ECS `Registry`
- Composed only of data components
- Controlled later by dedicated systems (movement, collision, weapon, etc.)

---

### Player

A player entity represents a controllable spaceship.  
It contains movement, input, collision, health, and weapon data.

#### Components
- `Transform` – world position
- `RigidBody` – physics movement
- `InputState` – player inputs
- `Weapon` – shooting behavior
- `BoxCollider` – collision detection
- `PlayerEntity` – health, speed, score, state

#### Creation Function
```cpp
Entity createPlayer(Registry& registry, const Vector2& startPos, int playerId) {
    Entity player = registry.spawn_entity();

    registry.add_component<Transform>(player, Transform{startPos});
    registry.add_component<RigidBody>(player, RigidBody{});
    registry.add_component<InputState>(player, InputState());
    registry.add_component<Weapon>(player, Weapon());
    registry.add_component<BoxCollider>(
        player, BoxCollider(PLAYER_SIZE.x, PLAYER_SIZE.y));
    registry.add_component<PlayerEntity>(
        player, PlayerEntity(playerId, 200.f, 100, 100));

    return player;
}

```
### Enemies

Enemies are AI-controlled entities that challenge the player throughout the game.  
They are created using helper functions and controlled entirely by ECS systems.

Each enemy type has distinct movement patterns and behaviors, but all enemies share
a common structure and lifecycle.

---

### Enemy Types

```cpp
enum class EnemyType {
    Basic,
    Zigzag,
    Chase
};

```
#### Components

- `Transform`

- `RigidBody`
- `BoxCollider`

```cpp
Entity createEnemy(Registry& registry, EnemyType type, const Vector2& startPos) {
    Entity enemy = registry.spawn_entity();

    registry.add_component<Transform>(enemy, Transform{startPos});
    registry.add_component<RigidBody>(enemy, RigidBody{});
    registry.add_component<BoxCollider>(
        enemy, BoxCollider(ENEMY_BASIC_SIZE.x, ENEMY_BASIC_SIZE.y));
    registry.add_component<Enemy>(enemy, Enemy{type, 150.f});

    return enemy;
}

```

### Boss
Bosses are special enemies with large hitboxes, high health, and multi-phase behaviors.
#### Components

`Transform`
`RigidBody`
`BoxCollider`

### Projectiles

Projectiles are spawned by weapons and interact with enemies, players, and bosses.

#### Components

`Transform`

`RigidBody`

`BoxCollider`

> **Note:**  
> The following sections provide **examples** of entity creation using helper functions.  
> Not all possible components or configurations are shown here.
>
> All entity creation helpers are implemented in:  
> `Helpers/EntityHelper.hpp`

## Movement System

PATH = Shared/system/Movement \

Movement in R-TYPE is fully managed by ECS systems.  
Entities do not move themselves; systems calculate velocities and update positions based on component data.  

> **Important:** To have movement functionality for any entity, it **must have the appropriate components registered** in the ECS:
> - `Transform` – position in the world  
> - `RigidBody` – stores velocity and movement data  
> - Additional components depending on entity type:
>   - `PlayerEntity` + `InputState` for players  
>   - `Enemy` for enemies  
>   - `Boss` for bosses  
>   - `Projectile` for projectiles  

---

### Player Movement

The player movement system reads input from `InputState` and applies it to the `RigidBody`.

- **Components involved:**
  - `PlayerEntity` – stores speed and player state
  - `InputState` – stores movement keys
  - `RigidBody` – stores velocity

Example logic from `player_movement_system`:

```cpp
for (auto&& [state, rb, player] : Zipper(states, rigidbodies, players)) {
    rb.velocity = {0.f, 0.f};
    if (state.moveLeft) rb.velocity.x = -player.speed;
    if (state.moveRight) rb.velocity.x = player.speed;
    if (state.moveUp) rb.velocity.y = -player.speed;
    if (state.moveDown) rb.velocity.y = player.speed;
}
```

### Enemies

Enemies are AI-controlled entities that challenge the player throughout the game.  
They are fully managed by ECS systems, which control movement, shooting, and interactions.

---

#### Enemy Types

R-TYPE enemies come in several varieties:

```cpp
enum class EnemyType {
    Basic,
    Zigzag,
    Chase
};
```
### Boss
 The `boss_movement_system` updates every frame:
   - Adjusts `RigidBody.velocity` based on boss type and phase
   - Spawns additional enemies if required (e.g., Battleship, FinalBoss)
   - Fires projectiles automatically at intervals
   - Updates phase timers for multi-phase bosses

---

#### Boss Types

R-TYPE implements several boss types:

```cpp
enum class BossType {
    BigShip,    // Large ship with sinusoidal movement, multi-phase
    Snake,      // Multi-segment serpent-like boss
    BydoEye,    // Stationary boss with heavy firing
    Battleship, // Spawns additional enemies
    FinalBoss   // Ultimate boss with unique attacks
};

enum class BossPhase {
    Phase1,
    Phase2,
    Phase3
};
```
### Example System Call

```cpp
boss_movement_system(registry, transforms, rigidbodies, bosses, deltaTime);
```

### Projectile Movement System

Projectiles are spawned by players, enemies, or bosses and move according to their velocity.  
The `Projectile_movement_system` updates all projectiles every frame.

---

#### Required Components

To function correctly, each projectile entity must have:

- `Transform` – position in the world  
- `RigidBody` – velocity  
- `Projectile` – stores damage, speed, lifetime, and owner information  

---

#### Behavior

- Updates the projectile's position based on its `RigidBody.velocity`  
- Increments `currentLife` to track lifetime  
- Automatically removes the projectile from the ECS registry when its lifetime exceeds the limit (e.g., 2 seconds)  

---

#### Example System Call

```cpp
Projectile_movement_system(transforms, rigidbodies, projectiles, registry, deltaTime);
```

> **Note:**  
> To add a new movement pattern, edit the movement system located at `Shared/system/Movement/Movement.hpp` and `Movement.cpp`.  
> 1. Define a new type in the relevant enum (`EnemyType` or `BossType`).  
> 2. Add a corresponding `case` in the movement system function (`enemy_movement_system` or `boss_movement_system`).  
> 3. Implement the velocity logic using the entity's components (`Transform`, `RigidBody`, and type-specific component).  
> 4. Test the new movement by spawning an entity of this type using the helper functions.



### Collision System

The collision system handles all interactions between entities, including players, enemies, bosses, projectiles, and items.  
It detects collisions, applies damage, and triggers events like item pickups.

---

#### Required Components

To participate in collisions, an entity must have:

- `Transform` – position in the world  
- `BoxCollider` – collision bounding box  
- Optional: `PlayerEntity`, `Enemy`, `Boss`, `Projectile`, or `Items` depending on the type  

> **Note:**  
> Entities missing `Transform` or `BoxCollider` will be ignored by the system.

---

#### Behavior

The `gamePlay_Collision_system` executes the following logic every frame:

1. **Collision Detection**
   - Iterates over all entities with `Transform` and `BoxCollider`
   - Uses axis-aligned bounding box (AABB) collision detection (`check_collision`)
   - Stores currently colliding pairs

2. **Collision Enter Events**
   - If a collision is new (not in previous frame), create a `Collision` component
   - Apply damage depending on entity types:
     - Projectile → Player/Enemy/Boss: apply projectile damage, then remove projectile
     - Player ↔ Enemy/Boss: apply contact damage to both entities
     - Player ↔ Item: mark item as picked up and remove from ECS
   - Logs debug messages for each collision enter

3. **Collision Exit Events**
   - Remove `Collision` components when entities are no longer colliding
   - Logs debug messages for collision exit

4. **Damage Application**
   - `apply_damage_to_entity` updates health (`current`) of the target entity
   - Handles invincibility timer for players
   - Kills entities when `current <= 0`
   - Updates player score if applicable

---

#### Example System Call

```cpp
gamePlay_Collision_system(registry, transforms, colliders, players, enemies, bosses, items, projectiles);
```
> **Note: Adding a New Entity to the Collision System**
>
> To include a new entity in the collision system, you need to modify the code located at:
>
> ```
> Shared/system/Collision/Collision.hpp
> Shared/system/Collision/Collision.cpp
> ```
>
> Steps to add collision for a new entity type:
>
> 1. Make sure your entity has the required components:
>    - `Transform` – world position
>    - `BoxCollider` – collision bounding box
>    - A gameplay-specific component (e.g., `PlayerEntity`, `Enemy`, `Boss`, `Projectile`, or `Items`)
>
> 2. Update the `get_entity_category` function in `Collision.cpp` to include your new component type. Example:
>
> ```cpp
> if (registry.get_components<MyNewComponent>()[entityId].has_value())
>     return CollisionCategory::MyNewCategory;
> ```
>
> 3. Optionally, handle damage or interactions in `gamePlay_Collision_system`. You can use the existing pattern:
>
> ```cpp
> if ((tagger == CollisionCategory::MyNewCategory && it == CollisionCategory::Player) ||
>     (tagger == CollisionCategory::Player && it == CollisionCategory::MyNewCategory)) {
>     apply_damage_to_entity(registry, entityB, myDamage, entityA);
> }
> ```
>
> 4. Ensure `apply_damage_to_entity` can handle your new entity type if it needs health/damage logic.
>
> 5. Rebuild and test by spawning an entity with your new component using the helper functions in `Helpers/EntityHelper.hpp`.
>
> **Tip:**  
> The collision system is modular. Adding a new entity only requires adding its component, categorizing it in `get_entity_category`, and optionally defining damage/interaction logic. The system automatically detects collisions via `Transform` and `BoxCollider`.

### Projectile System

The **Projectile System** in R-TYPE manages all projectiles spawned by players, enemies, or bosses.  
It handles their movement, lifetime, collision detection, and damage application.

---

#### Key Functions

1. **`projectile_lifetime_system`**  
   - Updates the lifetime of each active projectile every frame (`currentLife += deltaTime`).  
   - Automatically destroys projectiles when they exceed their `lifetime` by calling `registry.kill_entity`.

2. **`apply_projectile_damage`**  
   - Applies damage from a projectile to a target entity.  
   - Handles damage to `PlayerEntity`, `Enemy`, or `Boss`.  
   - Updates health, invincibility timer, and kills entities if health reaches 0.

3. **`projectile_collision_system`**  
   - Checks collisions between all projectiles and potential targets.  
   - Determines valid collisions based on the projectile owner type:
     - Player projectiles hit enemies and bosses.  
     - Enemy or boss projectiles hit players.  
   - Calls `apply_projectile_damage` on collision and removes the projectile from the ECS.

4. **`spawn_projectile` / `spawn_player_projectile`**  
   - Helper functions to create a projectile entity.  
   - Assigns `Transform`, `RigidBody`, `BoxCollider`, and `Projectile` components.  
   - Optionally, you can add `Sprite` or `Animation` components for rendering.

---

#### How to Contribute / Add Features

If you want to contribute or extend the projectile system:

1. **Add new projectile types**
   - Create a new component or enum for the projectile type if needed.  
   - Adjust `spawn_projectile` to set specific size, speed, direction, damage, or sprite.

2. **Modify collision rules**
   - Update `projectile_collision_system` to include new target types.  
   - Example: adding projectiles that can hit both enemies and other projectiles.

3. **Custom damage logic**
   - Extend `apply_projectile_damage` to handle special behaviors, e.g., explosions, piercing, or AoE effects.

4. **Projectile movement patterns**
   - Modify `RigidBody.velocity` in `spawn_projectile` or in a custom movement system.  
   - Example: make projectiles move in sine waves, zigzags, or accelerate over time.

> **Tip:**  
> The projectile system is modular. Each projectile is an ECS entity with components for data only. All logic (movement, collision, damage) is handled in systems. To add new functionality, you only need to add new components or extend the existing systems without modifying the core ECS structure.


### ServerGame

The `ServerGame` class manages the game logic on the server side for a multiplayer R-TYPE game.  
It handles player authentication, network messages, entity updates, and broadcasting the game state to clients.

---

#### Key Responsibilities

1. **Player Authentication (`HandleAuth`)**
   - Adds a new player to the lobby when they connect.
   - Assigns a starting position and spawns a `PlayerEntity` in the ECS registry.
   - Starts the game automatically when the lobby is full (e.g., 4 players).

2. **Network Callbacks (`SetupNetworkCallbacks`)**
   - Handles incoming messages from clients (`SetMessageCallback`), connections, and disconnections.
   - Decodes messages into `Event` objects and queues them for processing in the game loop.
   - Removes player entities when a client disconnects.

3. **Game Loop (`GameLoop`)**
   - Runs the main server-side loop, updating the game state at a fixed time step (~60 FPS).
   - Calls systems for movement, projectiles, collisions, waves, and physics.
   - Sends the current world state to all connected clients using `SendWorldStateToClients`.

4. **Event Processing (`ReceivePlayerInputs`)**
   - Pulls queued events from players.
   - Updates `InputState` components in the ECS for corresponding player entities.
   - Supports player actions like movement and firing weapons.

5. **World State Broadcasting (`SendWorldStateToClients`)**
   - Iterates through all relevant ECS components (players, enemies, bosses, projectiles).
   - Packs entity positions, health, and state into `GameState` messages.
   - Sends these messages to all clients to synchronize the game world.

6. **Systems Integration**
   - Calls movement systems (`player_movement_system`, `enemy_movement_system`, `boss_movement_system`).
   - Calls weapon and projectile systems.
   - Calls collision and bounds-checking systems.
   - Calls enemy wave system for spawning enemies periodically.

---

#### How a Contributor Can Extend or Contribute

1. **Adding New Player Actions**
   - Extend `PLAYER_INPUT` struct to include new actions.
   - Update `ReceivePlayerInputs` to process the new input flags.
   - Ensure these actions are reflected in ECS components (`PlayerEntity`, `InputState`).

2. **Implementing New Systems**
   - Add custom ECS systems (e.g., shields, power-ups, special attacks).
   - Call your system in `UpdateGameState` to integrate it with the main loop.

3. **Custom Network Messages**
   - Extend the `Event` and `Action` structures for new types of messages.
   - Implement corresponding encode/decode logic.
   - Use `SendAction` to broadcast new messages to clients.

4. **Gameplay Mechanics**
   - Modify enemy or boss behavior by adding new movement patterns, attacks, or AI logic.
   - Ensure all entity updates are sent to clients via `SendWorldStateToClients`.

5. **Adding New Entities**
   - Use the ECS registry to spawn new entities (projectiles, enemies, items, etc.).
   - Ensure each entity has the required components (`Transform`, `RigidBody`, `BoxCollider`, gameplay-specific components).
   - Include them in the appropriate systems for movement, collisions, and rendering.

6. **Debugging and Logging**
   - Utilize debug messages (`std::cout`) to track server events and collisions.
   - Add logging for new systems or gameplay mechanics to aid testing.

---

> **Tip:**  
> The server maintains authoritative game state. Clients only send inputs, and the server broadcasts the updated world state.  
> To contribute, focus on adding new ECS components, systems, or network messages. Do **not** rely on client-side calculations for game logic — everything authoritative happens in `ServerGame` on the server side.

