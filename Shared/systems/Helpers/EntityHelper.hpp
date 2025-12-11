// PlayerHelpers.hpp
#pragma once
#include "ecs/Registry.hpp"
#include "Player/PlayerController.hpp"
#include "Player/Enemy.hpp"
#include "Player/Boss.hpp"
#include "components/Physics2D.hpp"
#include "Collision/Items.hpp"
#include "Player/ProjectTile.hpp"

// tailles prédéfinies (pas constexpr)
static const Vector2 PLAYER_SIZE{32.f, 32.f};
static const Vector2 ENEMY_BASIC_SIZE{32.f, 32.f};
static const Vector2 ENEMY_BOSS_SIZE{128.f, 128.f};

enum class EntityType { Player, Enemy, Boss, Projectile };

// Helper pour créer un joueur
Entity createPlayer(Registry& registry, const Vector2& startPos, int playerId = 0) {
    Entity player = registry.spawn_entity();

    registry.add_component<Transform>(player, Transform{startPos});
    registry.add_component<RigidBody>(player, RigidBody{});
    registry.add_component<BoxCollider>(player, BoxCollider(PLAYER_SIZE.x, PLAYER_SIZE.y));
    registry.add_component<PlayerControlled>(player, PlayerControlled{
        playerId,
        Vector2{0.f, 0.f}, // input initial
        200.f,             // speed
        100,               // current
        100,               // max
        true,              // isAlive
        0.f,               // invtimer
        0,                 // weaponId
        false,             // hasForce
        0                  // score
    });

    return player;
}

// Helper pour créer un ennemi basique
Entity createEnemy(Registry& registry, EnemyType type, const Vector2& startPos) {
    static int enemyIdCounter = 0;
    Entity enemy = registry.spawn_entity();

    registry.add_component<Transform>(enemy, Transform{startPos});
    registry.add_component<RigidBody>(enemy, RigidBody{});
    registry.add_component<BoxCollider>(enemy, BoxCollider(ENEMY_BASIC_SIZE.x, ENEMY_BASIC_SIZE.y));
    registry.add_component<Enemy>(enemy, Enemy{type});
    registry.add_component<PlayerControlled>(enemy, PlayerControlled{
        -1,               // pas de playerId pour un ennemi
        Vector2{0.f, 0.f}, 
        100.f,            // vitesse de l'ennemi
        50,               // current
        50,               // max
        true,             // isAlive
        0.f,              // invtimer
        0,                // weaponId
        false,            // hasForce
        0                 // score
    });
    registry.add_component<int>(enemy, enemyIdCounter++);
    return enemy;
}


// Helper pour créer un boss
Entity createBoss(Registry& registry, BossType type, const Vector2& startPos) {
    static int bossIdCounter = 0; 
    Entity boss = registry.spawn_entity();

    registry.add_component<Transform>(boss, Transform{startPos});
    registry.add_component<RigidBody>(boss, RigidBody{});
    registry.add_component<BoxCollider>(boss, BoxCollider(ENEMY_BOSS_SIZE.x, ENEMY_BOSS_SIZE.y));
    registry.add_component<Boss>(boss, Boss{type});
    registry.add_component<PlayerControlled>(boss, PlayerControlled{
        -1,                // pas de playerId pour le boss
        Vector2{0.f, 0.f}, 
        50.f,              // vitesse du boss (moins rapide qu'un joueur)
        500,               // current
        500,               // max
        true,              // isAlive
        0.f,               // invtimer
        0,                 // weaponId
        false,             // hasForce
        0                  // score
    });
    registry.add_component<int>(boss, bossIdCounter++);
    return boss;
}


Entity createProjectile(Registry& registry,
                        const Vector2& startPos,
                        const Vector2& direction,
                        float speed,
                        Uint32 damage,
                        bool fromPlayer)
{
    Entity projectile = registry.spawn_entity();

    registry.add_component<Transform>(projectile, Transform(startPos));
    registry.add_component<RigidBody>(projectile, RigidBody());
    registry.add_component<BoxCollider>(projectile, BoxCollider(10.f, 10.f));
    registry.add_component<ProjectTile>(
        projectile,
        ProjectTile{
            speed,                       // speed
            direction.Normalized(),      // direction
            0.f,                         // timer
            true,                        // is_existing
            damage,                      // damage
            fromPlayer,                  // is_player_projectTile
            true                         // active
        }
    );

    return projectile;
}
