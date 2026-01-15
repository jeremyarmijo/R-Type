#include "systems/ChargedShoot.hpp"
#include <iostream>
#include "Helpers/EntityHelper.hpp"

void charged_shoot_system(Registry& registry, float deltaTime) {
  auto& players = registry.get_components<PlayerEntity>();
  auto& states = registry.get_components<InputState>();
  auto& transforms = registry.get_components<Transform>();

  for (auto&& [idx, player, state, transform] :
       IndexedZipper(players, states, transforms)) {
    if (!player.isAlive) continue;

    PlayerShoot& shoot = player.shoot;

    // ========== CHARGE (action2 = fire == 2) ==========
    if (state.action2) {
      if (!shoot.isCharging) {
        shoot.isCharging = true;
        shoot.chargeTime = 0.0f;
        std::cout << "[CHARGE] Player " << player.player_id
                  << " started charging" << std::endl;
      }

      shoot.chargeTime += deltaTime;
      if (shoot.chargeTime > shoot.maxChargeTime) {
        shoot.chargeTime = shoot.maxChargeTime;
      }

      int level = shoot.GetChargeLevel();
      if (level > 0) {
        std::cout << "[CHARGE] Player " << player.player_id
                  << " level=" << level << " time=" << shoot.chargeTime << "s"
                  << std::endl;
      }
    } else if (shoot.isCharging) {
      int chargeLevel = shoot.GetChargeLevel();

      std::cout << "[FIRE] chargeLevel=" << chargeLevel
                << " chargeTime=" << shoot.chargeTime << std::endl;
      if (chargeLevel > 0 || shoot.chargeTime >= 0.1f) {
        Vector2 spawnPos = {transform.position.x + 40.f, transform.position.y};
        Vector2 direction = {1.0f, 0.0f};
        float speed = 400.0f + chargeLevel * 100.0f;
        Uint32 baseDamage = 10 + (chargeLevel * 5);

        // ✅ Crée le projectile avec createProjectile
        Entity projectile = createProjectile(
            registry, spawnPos, direction, speed, baseDamage,
            true,        // isPlayerProjectile
            chargeLevel);

        std::cout << "[CHARGED SHOT] Player " << player.player_id
                  << " fired level " << chargeLevel << " projectile!"
                  << std::endl;
      }

      shoot.Reset();
    } else if (state.action1 && shoot.isCharging) {
      shoot.Reset();
    }
  }
}
