/*#include "PlayerHelpers.hpp"
#include "ecs/Registry.hpp"

int main() {
    Registry registry;

    // 1. On crée le joueur
    Vector2 playerStartPos{100.f, 200.f};
    Entity player = createPlayer(registry, playerStartPos);

    // 2. On crée un ennemi
    Vector2 enemyPos{400.f, 200.f};
    Entity enemy = createEnemy(registry, EnemyType::Basic, enemyPos);

    // 3. On peut créer un boss
    Vector2 bossPos{800.f, 100.f};
    Entity boss = createBoss(registry, BossType::BigShip, bossPos);

    // Ensuite, tu peux lancer tes systèmes (mouvement, physique, collisions...)
    while(gameRunning) {
        float deltaTime = getDeltaTime();

        player_movement_system(registry.get_components<PlayerControlled>(),
                               registry.get_components<Transform>(),
                               registry.get_components<RigidBody>(),
                               registry.get_components<InputState>(),
                               deltaTime);

        // ... autres systèmes
    }

    return 0;
}

}*/

// quand cree changer et detruire les entity Event.hpp Action.hpp
// create helper functions that automatically give entities components for you.
// eg: createBydosSlave() -> creates entity and assigns components with
// predefined values. pour creation d'entités
