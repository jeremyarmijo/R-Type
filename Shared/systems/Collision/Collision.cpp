// Copyright 2025 Dalia Guiz
#include "Collision/Collision.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Collision/Items.hpp"
#include "Player/Enemy.hpp"
#include "Player/PlayerEntity.hpp"
#include "Player/Projectile.hpp"
#include "ecs/Registry.hpp"
#include "systems/PhysicsSystem.hpp"

struct pair_hash {
  inline std::size_t operator()(
      const std::pair<size_t, size_t>& v) const noexcept {
    return v.first * 73856093u ^ v.second * 19349663u;
  }
};

static inline std::pair<size_t, size_t> ordered_pair(size_t a, size_t b) {
  if (a < b) return {a, b};
  return {b, a};
}

void gamePlay_Collision_system(
    Registry& registry, SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders, SparseArray<PlayerEntity>& players,
    SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
    SparseArray<Items>& items, SparseArray<Projectile>& projectiles) {
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

  static std::unordered_set<std::pair<size_t, size_t>, pair_hash>
      collisions_prev;

  for (const auto& pair : collisions_now) {
    if (collisions_prev.find(pair) == collisions_prev.end()) {
      size_t entityA = pair.first;
      size_t entityB = pair.second;

      CollisionCategory tagger = get_entity_category(entityA, registry);
      CollisionCategory it = get_entity_category(entityB, registry);

      Collision collision(Entity(entityA), Entity(entityB), tagger, it);

      std::cout << "[DEBUG][ENTER] Collision detected between " << entityA
                << " and " << entityB << std::endl;

      if (it == CollisionCategory::Player || it == CollisionCategory::Enemy ||
          it == CollisionCategory::Boss) {
        apply_damage(registry, collision, transforms, players, enemies, bosses,
                     projectiles);
      }

      if (it == CollisionCategory::Item) {
        if (items[collision.It].has_value()) {
          items[collision.It]->picked_up = true;
          registry.kill_entity(collision.It);
        }
      }

      if (tagger == CollisionCategory::Projectile) {
        registry.kill_entity(collision.tagger);
      }

      // ajouter le composant Collision uniquement si pas déjà présent
      if (!registry.has_component<Collision>(Entity(collision.It))) {
        registry.add_component<Collision>(Entity(collision.It),
                                          std::move(collision));
      }
    }
  }

  for (const auto& pair : collisions_prev) {
    if (collisions_now.find(pair) == collisions_now.end()) {
      size_t a = pair.first;
      size_t b = pair.second;

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

  collisions_prev = std::move(collisions_now);
}

CollisionCategory get_entity_category(size_t entityId, Registry& registry) {
  if (registry.get_components<PlayerEntity>()[entityId].has_value())
    return CollisionCategory::Player;
  if (registry.get_components<Enemy>()[entityId].has_value())
    return CollisionCategory::Enemy;
  if (registry.get_components<Projectile>()[entityId].has_value())
    return CollisionCategory::Projectile;
  if (registry.get_components<Items>()[entityId].has_value())
    return CollisionCategory::Item;
  if (registry.get_components<Boss>()[entityId].has_value())
    return CollisionCategory::Boss;

  return CollisionCategory::Unknown;
}

// Application des dégâts avec PlayerEntity
void apply_damage(Registry& registry, const Collision& collision,
                  SparseArray<Transform>& transforms,
                  SparseArray<PlayerEntity>& players,
                  SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
                  SparseArray<Projectile>& projectiles) {
  if (!players[collision.It].has_value()) return;

  auto& target = players[collision.It].value();

  int damage = 0;

  if (collision.taggerType == CollisionCategory::Projectile &&
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
