#include "systems/ProjectileSystem.hpp"

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <utility>

#include "Player/Enemy.hpp"
#include "ecs/Zipper.hpp"
#include "graphics/RenderComponents.hpp"
#include "systems/Collision/Collision.hpp"
#include "systems/PhysicsSystem.hpp"

// Forward declarations - les vraies structures sont dans le moteur
// struct Sprite {
//     std::string textureKey;
//     SDL_Rect sourceRect;
//     struct Vector2 { float x, y; };
//     Vector2 pivot;
//     int layer;
//     Sprite(const std::string& key, SDL_Rect src = {0, 0, 0, 0},
//            Vector2 piv = {0.5f, 0.5f}, int lyr = 0)
//         : textureKey(key), sourceRect(src), pivot(piv), layer(lyr) {}
// };

// struct Animation {
//     std::string currentAnimation;
//     float currentTime;
//     int currentFrame;
//     bool isPlaying;
//     explicit Animation(const std::string& anim = "", bool playing = false)
//         : currentAnimation(anim), currentTime(0),
//         currentFrame(0), isPlaying(playing) {}
// };

void projectile_lifetime_system(Registry& registry,
                                SparseArray<Projectile>& projectiles,
                                float deltaTime) {
  for (auto&& [idx, projectile] : IndexedZipper(projectiles)) {
    if (!projectile.isActive) continue;

    projectile.currentLife += deltaTime;

    if (projectile.currentLife >= projectile.lifetime) {
      registry.kill_entity(Entity(idx));
    }
  }
}

void apply_projectile_damage(Registry& registry, size_t targetId,
                             float damage) {
  auto& players = registry.get_components<PlayerEntity>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();

  if (targetId < players.size() && players[targetId].has_value()) {
    auto& player = players[targetId].value();
    if (player.invtimer <= 0.0f) {
      player.current -= static_cast<int>(damage);
      if (player.current <= 0) {
        player.isAlive = false;
        registry.kill_entity(Entity(targetId));
      } else {
        player.invtimer = 0.5f;
      }
    }
    return;
  }
  if (targetId < enemies.size() && enemies[targetId].has_value()) {
    auto& enemy = enemies[targetId].value();
    enemy.current -= static_cast<int>(damage);
    if (enemy.current <= 0) {
      registry.kill_entity(Entity(targetId));
    }
    return;
  }
  if (targetId < bosses.size() && bosses[targetId].has_value()) {
    auto& boss = bosses[targetId].value();
    boss.current -= static_cast<int>(damage);
    if (boss.current <= 0) {
      registry.kill_entity(Entity(targetId));
    }
    return;
  }
}

void projectile_collision_system(Registry& registry,
                                 const SparseArray<Transform>& transforms,
                                 const SparseArray<BoxCollider>& colliders,
                                 SparseArray<Projectile>& projectiles) {
  auto& enemies = registry.get_components<Enemy>();
  auto& players = registry.get_components<PlayerEntity>();
  auto& bosses = registry.get_components<Boss>();
  std::vector<size_t> toKill;

  auto get_owner_type = [&](size_t ownerId) -> std::optional<std::string> {
    if (ownerId < players.size() && players[ownerId].has_value()) {
      return "Player";
    }
    if (ownerId < enemies.size() && enemies[ownerId].has_value()) {
      return "Enemy";
    }
    if (ownerId < bosses.size() && bosses[ownerId].has_value()) {
      return "Boss";
    }
    return std::nullopt;
  };

  for (auto&& [projIdx, projectile, projTransform, projCollider] :
       IndexedZipper(projectiles, transforms, colliders)) {
    if (!projectile.isActive) continue;

    std::optional<std::string> ownerType = get_owner_type(projectile.ownerId);
    if (!ownerType.has_value()) continue;

    for (auto&& [targetIdx, targetTransform, targetCollider] :
         IndexedZipper(transforms, colliders)) {
      if (projIdx == targetIdx) continue;
      if (targetIdx == projectile.ownerId) continue;

      bool isTargetPlayer =
          (targetIdx < players.size() && players[targetIdx].has_value());
      bool isTargetEnemy =
          (targetIdx < enemies.size() && enemies[targetIdx].has_value());
      bool isTargetBoss =
          (targetIdx < bosses.size() && bosses[targetIdx].has_value());

      bool validCollision = false;

      if (*ownerType == "Player" && isTargetEnemy) {
        validCollision = true;
      } else if (*ownerType == "Enemy" && isTargetPlayer) {
        validCollision = true;
      } else if (*ownerType == "Player" && isTargetBoss) {
        validCollision = true;
      }

      if (validCollision && check_collision(projTransform, projCollider,
                                            targetTransform, targetCollider)) {
        projectile.isActive = false;
        apply_projectile_damage(registry, targetIdx, projectile.damage);
        toKill.push_back(projIdx);
        break;
      }
    }
  }

  // Supprime tous les projectiles touchés **après** avoir appliqué tous les
  // dégâts
  for (size_t e : toKill) {
    if (e < projectiles.size()) {
      registry.kill_entity(Entity(e));
    }
  }
}

Entity spawn_projectile(Registry& registry, Vector2 position, Vector2 direction,
                        float speed, size_t ownerId) {
  Entity projectile = registry.spawn_entity();

  // Transform
  registry.emplace_component<Transform>(projectile, position);

  // // Sprite avec la texture du projectile (blueShoot.png)
  // registry.emplace_component<Sprite>(projectile, "projectile_player",
  //                                    SDL_Rect{0, 0, 19, 6}, Vector2{0.5f,
  //                                    0.5f}, 2);

  // // Animation pour animer entre les deux frames
  // registry.emplace_component<Animation>(projectile, "projectile_player_anim",
  //                                       true);

  // Physics - projectile avec vélocité fixe
  Vector2 velocity = direction.Normalized() * speed * 2;
  RigidBody rb(0.0f, 0.0f, false);  // mass=0, restitution=0, isStatic=false
  rb.velocity = velocity;
  registry.emplace_component<RigidBody>(projectile, std::move(rb));

  // Collider adapté à la taille du sprite (19x6)
  registry.emplace_component<BoxCollider>(projectile, 19.0f, 6.0f);

  // Projectile component
  registry.emplace_component<Projectile>(projectile, 10.0f, speed, direction,
                                         3.0f, ownerId);

  return projectile;
}
