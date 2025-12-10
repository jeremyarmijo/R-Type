// Copyright 2025 Dalia Guiz
#include <utility>
#include <vector>
#include "Collision/Collision.hpp"
#include "systems/PhysicsSystem.hpp"
#include "Player/PlayerController.hpp"
#include "Player/Enemy.hpp"
#include "Player/ProjectTile.hpp"
#include "Collision/CollisionCotroller.hpp"
#include "Player/Boss.hpp"
#include "Collision/Items.hpp"

// Collision system avec PlayerControlled à la place de Health
void gamePlay_Collision_system(
    Registry& registry,
    SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders,
    SparseArray<PlayerControlled>& players,
    SparseArray<Items>& items,
    SparseArray<ProjectTile>& projectiles)   // <-- ajouté ici
{
    std::vector<size_t> colli_entities; 
    for (auto&& [ix, transform, collider] : IndexedZipper(transforms, colliders)) {
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

            if (!check_collision(*transformA, *colliderA, *transformB, *colliderB))
                continue;

            CollisionCategory tagger = get_entity_category(entityA, registry);
            CollisionCategory it     = get_entity_category(entityB, registry);

            Collision collision(
                Entity(entityA),
                Entity(entityB),
                tagger,
                it
            );

            // Appliquer les dégâts si c'est un personnage
            if (it == CollisionCategory::Player ||
                it == CollisionCategory::Enemy ||
                it == CollisionCategory::Boss) {

                apply_damage(registry, collision, transforms, players, projectiles);

            }

            registry.add_component<Collision>(collision.It, std::move(collision));

            // Gestion des items
            if (it == CollisionCategory::Item) {
                if (items[collision.It].has_value()) {
                    items[collision.It]->picked_up = true;
                    registry.kill_entity(collision.It);
                }
            }

            // Projectile détruit après collision
            if (tagger == CollisionCategory::ProjectTile) {
                registry.kill_entity(collision.tagger);
            }
        }
    }
}

// Identification du type d'entité
CollisionCategory get_entity_category(size_t entityId, Registry& registry) {
    if (registry.get_components<PlayerControlled>()[entityId].has_value())
        return CollisionCategory::Player;
    if (registry.get_components<Enemy>()[entityId].has_value())
        return CollisionCategory::Enemy;
    if (registry.get_components<ProjectTile>()[entityId].has_value())
        return CollisionCategory::ProjectTile;
    if (registry.get_components<Items>()[entityId].has_value())
        return CollisionCategory::Item;
    if (registry.get_components<Boss>()[entityId].has_value())
        return CollisionCategory::Boss;

    return CollisionCategory::Unknown;
}

/* Calcul des dégâts
int compute_damage(CollisionCategory tagger, CollisionCategory it) {
    if (tagger == CollisionCategory::Player && it == CollisionCategory::Enemy) return 10;
    if (tagger == CollisionCategory::Enemy && it == CollisionCategory::Player) return 5;
    if (tagger == CollisionCategory::ProjectTile && it == CollisionCategory::Enemy) return 15;
    if (tagger == CollisionCategory::Boss && it == CollisionCategory::Player) return 30;
    if (tagger == CollisionCategory::Player && it == CollisionCategory::Boss) return 5;
    return 0;
}*/

// Application des dégâts avec PlayerControlled
void apply_damage(
    Registry& registry,
    const Collision& collision,
    SparseArray<Transform>& transforms,
    SparseArray<PlayerControlled>& players,
    SparseArray<ProjectTile>& projectiles) 
{
    // Vérifier que la cible a un PlayerControlled
    if (!players[collision.It].has_value())
        return;

    auto& target = players[collision.It].value();

    int damage = 0;

    // Si ce qui touche est un projectile :
    if (collision.taggerType == CollisionCategory::ProjectTile) {
        if (projectiles[collision.tagger].has_value()) {
            damage = projectiles[collision.tagger]->damage;
        }
    }

    // Appliquer les dégâts
    target.current -= damage;

    // Vérifier la mort
    if (target.current <= 0) {

        target.isAlive = false;

        Vector2 deadPos{};
        if (transforms[collision.It].has_value()) {
            deadPos = transforms[collision.It]->position;
        }

        Death died(
            collision.It,
            collision.tagger,
            collision.itType,
            deadPos
        );

        registry.add_component<Death>(collision.It, std::move(died));
        registry.kill_entity(collision.It);
    }
}
