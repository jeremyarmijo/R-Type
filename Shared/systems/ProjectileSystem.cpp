#include <SDL2/SDL.h>

#include <string>
#include <utility>

#include "systems/ProjectileSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "ecs/Zipper.hpp"

// Forward declarations - les vraies structures sont dans le moteur
struct Sprite {
    std::string textureKey;
    SDL_Rect sourceRect;
    struct Vector2 { float x, y; };
    Vector2 pivot;
    int layer;
    Sprite(const std::string& key, SDL_Rect src = {0, 0, 0, 0},
           Vector2 piv = {0.5f, 0.5f}, int lyr = 0)
        : textureKey(key), sourceRect(src), pivot(piv), layer(lyr) {}
};

struct Animation {
    std::string currentAnimation;
    float currentTime;
    int currentFrame;
    bool isPlaying;
    explicit Animation(const std::string& anim = "", bool playing = false)
        : currentAnimation(anim), currentTime(0),
        currentFrame(0), isPlaying(playing) {}
};

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

void projectile_collision_system(Registry& registry,
                                 const SparseArray<Transform>& transforms,
                                 const SparseArray<BoxCollider>& colliders,
                                 SparseArray<Projectile>& projectiles) {
    for (auto&& [projIdx, projectile, projTransform, projCollider] :
         IndexedZipper(projectiles, transforms, colliders)) {
        if (!projectile.isActive) continue;

        for (auto&& [targetIdx, targetTransform, targetCollider] :
             IndexedZipper(transforms, colliders)) {
            // Ignorer le projectile lui-même
            if (projIdx == targetIdx) continue;

            // Ignorer le propriétaire du projectile (celui qui l'a tiré)
            if (targetIdx == projectile.ownerId) continue;

            if (check_collision(projTransform, projCollider,
                              targetTransform, targetCollider)) {
                projectile.isActive = false;
                registry.kill_entity(Entity(projIdx));

                // TODO(ZiadBengherabi) apply damage to target entity
                break;
            }
        }
    }
}

Entity spawn_projectile(Registry& registry,
                       Vector2 position,
                       Vector2 direction,
                       float speed,
                       size_t ownerId) {
    Entity projectile = registry.spawn_entity();

    // Transform
    registry.emplace_component<Transform>(projectile, position);

    // Sprite avec la texture du projectile (blueShoot.png)
    registry.emplace_component<Sprite>(projectile, Sprite("projectile",
                                       SDL_Rect{0, 0, 19, 6},
                                       Sprite::Vector2{0.5f, 0.5f}, 2));

    // Animation pour animer entre les deux frames
    registry.emplace_component<Animation>(projectile,
                                        Animation("projectile_anim", true));

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
