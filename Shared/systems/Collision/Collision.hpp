// Copyright 2025 Dalia Guiz
#pragma once
#include "components/Physics2D.hpp"
#include "ecs/Registry.hpp"
#include "components/Collision/CollisionController.hpp"
#include "components/Player/PlayerEntity.hpp"
#include "ecs/Zipper.hpp"


void gamePlay_Collision_system(Registry& registry,
    SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders,
    SparseArray<Health>& healths,
    SparseArray<Items>& items);

CollisionCategory get_entity_category(size_t entityId, Registry& registry);

void apply_damage(Registry& registry,
    const Collision& collision,
    SparseArray<Transform>& transforms,
    SparseArray<Health>& healthComponents);


int compute_damage(CollisionCategory tagger, CollisionCategory it);
