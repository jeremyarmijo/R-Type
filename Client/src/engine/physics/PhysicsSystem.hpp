#pragma once
#include "ecs/Registry.hpp"
#include "ecs/Zipper.hpp"
#include "physics/Physics2D.hpp"

void physics_movement_system(Registry& registry,
                             SparseArray<Transform>& transforms,
                             SparseArray<RigidBody>& rigidbodies,
                             float deltaTime,
                             const Vector2& gravity = {0, 9.81f});

void collision_detection_system(Registry& registry,
                                const SparseArray<Transform>& transforms,
                                const SparseArray<BoxCollider>& colliders,
                                SparseArray<RigidBody>& rigidbodies);

void physics_system(Registry& registry, float deltaTime,
                    const Vector2& gravity = {0, 9.81f});

bool check_collision(const Transform& transformA, const BoxCollider& colliderA,
                     const Transform& transformB, const BoxCollider& colliderB);

void resolve_collision(RigidBody& rigidbodyA, RigidBody& rigidbodyB,
                       const Transform& transformA,
                       const Transform& transformB);
