#ifndef SHARED_SYSTEMS_FORCECTRL_HPP_
#define SHARED_SYSTEMS_FORCECTRL_HPP_

#include "Player/PlayerEntity.hpp"
#include "components/BossPart.hpp"
#include "components/Force.hpp"
#include "physics/Physics2D.hpp"
#include "components/Player/Boss.hpp"
#include "components/Player/Enemy.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/Zipper.hpp"
#include "input/InputSubsystem.hpp"

void force_control_system(Registry& registry, SparseArray<Force>& forces,
                          SparseArray<InputState>& states,
                          SparseArray<Transform>& transforms);
void force_collision_system(
    Registry& registry, SparseArray<Transform>& transforms,
    SparseArray<BoxCollider>& colliders, SparseArray<Force>& forces,
    SparseArray<Enemy>& enemies, SparseArray<Boss>& bosses,
    SparseArray<BossPart>& bossParts, SparseArray<Projectile>& projectiles);

#endif  // SHARED_SYSTEMS_FORCECTRL_HPP_
