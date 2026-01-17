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
#include "components/BossPart.hpp"
#include "components/Force.hpp"
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

void gamePlay_Collision_system(Registry& registry,
                               SparseArray<Transform>& transforms,
                               SparseArray<BoxCollider>& colliders,
                               SparseArray<PlayerEntity>& players,
                               SparseArray<Enemy>& enemies,
                               SparseArray<Boss>& bosses) {
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

      if ((tagger == CollisionCategory::Player &&
           it == CollisionCategory::Enemy) ||
          (tagger == CollisionCategory::Enemy &&
           it == CollisionCategory::Player) ||
          (tagger == CollisionCategory::Player &&
           it == CollisionCategory::Boss) ||
          (tagger == CollisionCategory::Boss &&
           it == CollisionCategory::Player)) {
        // A deals damage to B
        int damage_A_to_B =
            (tagger == CollisionCategory::Enemy && enemies[entityA].has_value())
                ? enemies[entityA]->contact_damage
            : (tagger == CollisionCategory::Boss && bosses[entityA].has_value())
                ? bosses[entityA]->contact_damage
                : 0;
        if (damage_A_to_B > 0) {
          apply_damage_to_entity(registry, entityB, damage_A_to_B, entityA);
        }

        // B deals damage to A
        int damage_B_to_A =
            (it == CollisionCategory::Enemy && enemies[entityB].has_value())
                ? enemies[entityB]->contact_damage
            : (it == CollisionCategory::Boss && bosses[entityB].has_value())
                ? bosses[entityB]->contact_damage
                : 0;
        if (damage_B_to_A > 0) {
          apply_damage_to_entity(registry, entityA, damage_B_to_A, entityB);
        }
      }

      // if (it == CollisionCategory::Item) {
      //   if (items[collision.It].has_value()) {
      //     items[collision.It]->picked_up = true;
      //     registry.kill_entity(collision.It);
      //   }
      // }
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
    }
  }

  collisions_prev = std::move(collisions_now);
}

CollisionCategory get_entity_category(size_t entityId, Registry& registry) {
  if (registry.get_components<PlayerEntity>()[entityId].has_value())
    return CollisionCategory::Player;
  if (registry.get_components<Enemy>()[entityId].has_value())
    return CollisionCategory::Enemy;
  // if (registry.get_components<Projectile>()[entityId].has_value())
  //   return CollisionCategory::Projectile;
  // if (registry.get_components<Items>()[entityId].has_value())
  //   return CollisionCategory::Item;
  if (registry.get_components<Boss>()[entityId].has_value())
    return CollisionCategory::Boss;
  if (registry.get_components<BossPart>()[entityId].has_value())
    return CollisionCategory::BossPart;
  if (registry.get_components<Force>()[entityId].has_value())
    return CollisionCategory::Force;
  return CollisionCategory::Unknown;
}

void apply_damage_to_entity(Registry& registry, size_t targetId, float damage,
                            size_t attackerId) {
  auto& players = registry.get_components<PlayerEntity>();
  auto& enemies = registry.get_components<Enemy>();
  auto& bosses = registry.get_components<Boss>();
  auto& bossParts = registry.get_components<BossPart>();

  if (targetId < players.size() && players[targetId].has_value()) {
    auto& player = players[targetId].value();
    if (player.invtimer <= 0.0f) {
      player.current -= static_cast<int>(damage);
      if (player.current <= 0) {
        player.isAlive = false;
        std::cout << "[DEATH] Player " << player.player_id 
                << " died with score: " << player.score << std::endl;
        registry.kill_entity(Entity(targetId));
      } else {
        player.invtimer = 0.5f;
      }
    }
    return;
  }

  // Si c'est un ennemi qui reçoit les dégâts
  // if (targetId < enemies.size() && enemies[targetId].has_value()) {
  //   auto& enemy = enemies[targetId].value();
  //   enemy.current -= static_cast<int>(damage);

  //   if (enemy.current <= 0) {
  //     // Ajouter le score au joueur attaquant
  //     if (attackerId < players.size() && players[attackerId].has_value()) {
  //       players[attackerId]->score += enemy.scoreValue;
  //     }
  //     registry.kill_entity(Entity(targetId));
  //   }
  //   return;
  // }

  // if (targetId < bosses.size() && bosses[targetId].has_value()) {
  //   auto& boss = bosses[targetId].value();
  //   boss.current -= static_cast<int>(damage);

  //   std::cout << "Boss " << targetId << " hit. Health: " << boss.current
  //             << std::endl;

  //   if (boss.current <= 0) {
  //     // if (attackerId < players.size() && players[attackerId].has_value())
  //     {
  //     // players[attackerId]->score += boss.scoreValue;
  //     //}
  //     registry.kill_entity(Entity(targetId));
  //   }
  //   return;
  // }
}
