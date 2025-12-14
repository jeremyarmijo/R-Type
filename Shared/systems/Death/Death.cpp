/*#include "Death.hpp"
#include "../../components/Player/PlayerEntity.hpp"
#include "../../components/Collision/CollisionCotroller.hpp"

void death_system(Registry& registry) {
    auto& deaths = registry.get_components<Death>();

    for (size_t i = 0; i < deaths.size(); ++i) {
        if (!deaths[i].has_value()) continue;
        Death& death = *deaths[i];

        //spawn_explosion(death.position);

        if (death.type == CollisionCategory::Enemy || death.type == CollisionCategory::Boss) {
            auto& players = registry.get_components<PlayerId>();
            if (players[death.killer].has_value()) {
                players[death.killer]->score += 10; // score arbitraire
            }
        }

       // if (death.type == CollisionCategory::Player) {
         //   schedule_respawn(death.deadEntity);
       // }

        registry.remove_component<Death>(death.deadEntity);
    }
}*/
