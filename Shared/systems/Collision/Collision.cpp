// Copyright 2025 Dalia Guiz
#include <utility>
#include <vector>
#include "Collision/Collision.hpp"
#include "systems/PhysicsSystem.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Enemy.hpp"
#include "Player/Projectile.hpp"
#include "Collision/CollisionController.hpp"
#include "Player/Boss.hpp"

void gamePlay_Collision_system(Registry& registry,
    SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders,
    SparseArray<Health>& healths,
    SparseArray<Items>& items) {
    std::vector<size_t> colli_entities;
    for (auto&& [ix, transform, collider] :
    IndexedZipper(transforms, colliders)) {
        colli_entities.push_back(ix);
    }

    for (size_t i = 0; i < colli_entities.size(); ++i) {
        for (size_t j = i + 1; j < colli_entities.size(); ++j) {
            size_t entityA = colli_entities[i];
            size_t entityB = colli_entities[j];

            const auto& transformA = transforms[entityA];
            const auto& colliderA  = colliders[entityA];
            const auto& transformB = transforms[entityB];
            const auto& colliderB  = colliders[entityB];

            if (!transformA || !colliderA || !transformB || !colliderB)
                continue;

            if (!check_collision(*transformA, *colliderA, *transformB,
            *colliderB))
                continue;

            CollisionCategory tagger = get_entity_category(entityA, registry);
            CollisionCategory it     = get_entity_category(entityB, registry);
            Collision collision(
                Entity(entityA),
                Entity(entityB),
                tagger,
                it,
                compute_damage(tagger, it));

            if (it == CollisionCategory::Player ||
            it == CollisionCategory::Enemy ||
            it == CollisionCategory::Boss) {
                apply_damage(registry, collision, transforms, healths);
            }
            registry.add_component<Collision>
            (collision.It, std::move(collision));

            if (it == CollisionCategory::Item) {
                if (items[collision.It].has_value()) {
                    items[collision.It]->picked_up = true;
                    registry.kill_entity(collision.It);
                }
            }
            if (tagger == CollisionCategory::Projectile) {
                registry.kill_entity(collision.tagger);
            }
        }
    }
}



CollisionCategory get_entity_category(size_t entityId, Registry& registry) {
    if (registry.get_components<PlayerEntity>().operator[]
    (entityId).has_value()) {
        return CollisionCategory::Player;
    }
    if (registry.get_components<Enemy>().operator[](entityId).has_value()) {
        return CollisionCategory::Enemy;
    }
    if (registry.get_components<Projectile>().operator[](entityId).has_value())
        return CollisionCategory::Projectile;
    if (registry.get_components<Items>().operator[](entityId).has_value())
        return CollisionCategory::Item;
    if (registry.get_components<Boss>().operator[](entityId).has_value())
        return CollisionCategory::Boss;
    return CollisionCategory::Unknown;
}


int compute_damage(CollisionCategory tagger, CollisionCategory it) {
    if (tagger == CollisionCategory::Player && it == CollisionCategory::Enemy) {
        return 10;  // joueur frappe ennemi
    }
    if (tagger == CollisionCategory::Enemy && it == CollisionCategory::Player) {
        return 5;  // ennemi frappe joueur
    }
    if (tagger == CollisionCategory::Projectile &&
    it == CollisionCategory::Enemy) {
        return 15;  // projectile touche ennemi
    }
    if (tagger == CollisionCategory::Boss && it == CollisionCategory::Player) {
        return 30;  // bosse frappe player
    }
    if (tagger == CollisionCategory::Player && it == CollisionCategory::Boss)
        return 5;
    return 0;
}

void apply_damage(Registry& registry,
    const Collision& collision,
    SparseArray<Transform>& transforms,
    SparseArray<Health>& healthComponents) {
    if (healthComponents[collision.It].has_value()) {
        healthComponents[collision.It]->current -= collision.damage;

    if (healthComponents[collision.It]->current <= 0) {
        healthComponents[collision.It]->isAlive = false;
    Vector2 deadPos{};
    if (transforms[collision.It].has_value()) {
        deadPos = transforms[collision.It]->position;
    }
        Death died(
            collision.It,
            collision.tagger,
            collision.itType,
            deadPos);
    registry.add_component<Death>(collision.It, std::move(died));
    registry.kill_entity(collision.It);
        }
    }
}
