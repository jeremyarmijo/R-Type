// Copyright 2025 Dalia Guiz
#include "Collision/Collision.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>
#include "Collision/CollisionCotroller.hpp"
#include "Collision/Items.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"
#include "systems/PhysicsSystem.hpp"

// Hash pour pair<size_t,size_t>
struct pair_hash {
  inline std::size_t operator()(
      const std::pair<size_t, size_t>& v) const noexcept {
    // mix simple ; évite collisions symétriques => on stocke (min,max)
    return v.first * 73856093u ^ v.second * 19349663u;
  }
};

// Helper pour normaliser l'ordre d'une paire (min,max)
static inline std::pair<size_t, size_t> ordered_pair(size_t a, size_t b) {
  if (a < b) return {a, b};
  return {b, a};
}

void gamePlay_Collision_system(
    Registry& registry, SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders, SparseArray<PlayerControlled>& players,
    SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
    SparseArray<Items>& items, SparseArray<ProjectTile>& projectiles) {
  // 1) construire l'ensemble des paires en collision cette frame
  std::unordered_set<std::pair<size_t, size_t>, pair_hash> collisions_now;
  std::vector<size_t> colli_entities;
  for (auto&& [ix, transform, collider] :
       IndexedZipper(transforms, colliders)) {
    (void)transform;
    (void)collider;
    colli_entities.push_back(ix);
  }

  for (size_t i = 0; i < colli_entities.size(); ++i) {
    for (size_t j = i + 1; j < colli_entities.size(); ++j) {
      size_t a = colli_entities[i];
      size_t b = colli_entities[j];

      const auto& ta = transforms[a];
      const auto& ca = colliders[a];
      const auto& tb = transforms[b];
      const auto& cb = colliders[b];

      if (!ta || !ca || !tb || !cb) continue;

      if (check_collision(*ta, *ca, *tb, *cb)) {
        collisions_now.insert(ordered_pair(a, b));
      }
    }
  }

  // 2) static set qui garde l'état des collisions de la frame précédente
  static std::unordered_set<std::pair<size_t, size_t>, pair_hash>
      collisions_prev;

  // 3) traiter les nouvelles collisions (enter)
  for (const auto& pair : collisions_now) {
    if (collisions_prev.find(pair) == collisions_prev.end()) {
      // nouvelle collision (enter)
      size_t entityA = pair.first;
      size_t entityB = pair.second;

      // Choix de convention : tagger = A, it = B (ou inverse selon toi)
      CollisionCategory tagger = get_entity_category(entityA, registry);
      CollisionCategory it = get_entity_category(entityB, registry);

      // construit objet collision (utilise Entity wrapper si besoin)
      Collision collision(Entity(entityA), Entity(entityB), tagger, it);

      std::cout << "[DEBUG][ENTER] Collision detected between " << entityA
                << " and " << entityB << std::endl;

      // Appliquer dégâts si cible est personnage (on utilise apply_damage)
      if (it == CollisionCategory::Player || it == CollisionCategory::Enemy ||
          it == CollisionCategory::Boss) {
        apply_damage(registry, collision, transforms, players, enemies, bosses,
                     projectiles);
      }

      // Items
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

      // ajouter le composant Collision uniquement si pas déjà présent
      if (!registry.has_component<Collision>(Entity(collision.It))) {
        registry.add_component<Collision>(Entity(collision.It),
                                          std::move(collision));
      }
    }
  }

  // 4) traiter les collisions qui ont disparu (exit) => on retire le composant
  // Collision si on veut
  for (const auto& pair : collisions_prev) {
    if (collisions_now.find(pair) == collisions_now.end()) {
      // pair était présente avant, plus maintenant => exit
      size_t a = pair.first;
      size_t b = pair.second;

      // supprime le composant Collision des deux côtés si présent (ou juste
      // celui qui le porte)
      if (registry.has_component<Collision>(Entity(a))) {
        try {
          registry.remove_component<Collision>(Entity(a));
        } catch (...) {
        }
      }
      if (registry.has_component<Collision>(Entity(b))) {
        try {
          registry.remove_component<Collision>(Entity(b));
        } catch (...) {
        }
      }

      std::cout << "[DEBUG][EXIT] Collision ended between " << a << " and " << b
                << std::endl;
    }
  }

  // 5) swap prev <- now pour la frame suivante
  collisions_prev = std::move(collisions_now);
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

// Application des dégâts avec PlayerControlled
void apply_damage(Registry& registry, const Collision& collision,
                  SparseArray<Transform>& transforms,
                  SparseArray<PlayerControlled>& players,
                  SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
                  SparseArray<ProjectTile>& projectiles) {
  // Vérifier que la cible a un PlayerControlled
  if (!players[collision.It].has_value()) return;

  auto& target = players[collision.It].value();

  int damage = 0;

  if (collision.taggerType == CollisionCategory::ProjectTile &&
      projectiles[collision.tagger].has_value()) {
    damage = projectiles[collision.tagger]->damage;
  }

  if (players[collision.It].has_value()) {
    players[collision.It]->current -= damage;
    if (players[collision.It]->current <= 0) {
      players[collision.It]->isAlive = false;
      registry.kill_entity(collision.It);
    }
  } else if (enemies[collision.It].has_value()) {
    enemies[collision.It]->current -= damage;
    if (enemies[collision.It]->current <= 0) {
      registry.kill_entity(collision.It);
    }
  } else if (bosses[collision.It].has_value()) {
    bosses[collision.It]->current -= damage;
    if (bosses[collision.It]->current <= 0) {
      registry.kill_entity(collision.It);
    }
  }
}
