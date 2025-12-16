// #pragma once
#include "systems/WeaponSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "ecs/Zipper.hpp"

bool needs_reload(const Weapon& weapon) {
    return weapon.magazineSize != -1 &&
           (weapon.currentAmmo % weapon.magazineSize == 0) &&
           weapon.reloadTime > 0.0f;
}

bool can_fire(const Weapon& weapon) {
    return weapon.timeSinceLastShot >= (1.0f / weapon.fireRate) &&
           (weapon.magazineSize == -1 || weapon.currentAmmo > 0) &&
           !needs_reload(weapon);
}

void consume_ammo(Weapon& weapon) {
    if (weapon.magazineSize != -1 && weapon.currentAmmo > 0) {
        weapon.currentAmmo--;
    }
}

void start_reload(Weapon& weapon) {
    if (weapon.magazineSize != -1 && weapon.currentAmmo < weapon.magazineSize) {
        weapon.reloadTime = weapon.reloadTime;  // set reload time
    }
}

void weapon_cooldown_system(Registry& registry,
                            SparseArray<Weapon>& weapons,
                            float deltaTime) {
    for (auto&& [idx, weapon] : IndexedZipper(weapons)) {
        weapon.timeSinceLastShot += deltaTime;
    }
}

void weapon_reload_system(Registry& registry,
                           SparseArray<Weapon>& weapons,
                           float deltaTime) {
    for (auto&& [idx, weapon] : IndexedZipper(weapons)) {
        if (needs_reload(weapon)) {
            weapon.reloadTime -= deltaTime;
            if (weapon.reloadTime <= 0.0f) {
                weapon.currentAmmo = weapon.magazineSize;
                weapon.reloadTime = -1.0f;  // reset reload time
            }
        }
    }
}

void weapon_firing_system(Registry& registry,
                          SparseArray<Weapon>& weapons,
                          SparseArray<Transform>& transforms,
                          const std::function<bool(size_t)>& is_firing_input,
                          float deltaTime) {
    for (auto&& [idx, weapon, transform] : IndexedZipper(weapons, transforms)) {
        if (!is_firing_input(idx)) {
            continue;
        }

        if (can_fire(weapon)) {
             // Spawn projectile
            Vector2 projectilePos = transform.position
                                    + Vector2{3.0f, 0.0f};  // Example offset
            Vector2 projectileDir = {1.0f, 0.0f};  // Example direction
            spawn_projectile(registry, projectilePos, projectileDir,
                             weapon.projectileType.speed, idx);

            weapon.timeSinceLastShot = 0.0f;
            consume_ammo(weapon);
        }
    }
}
