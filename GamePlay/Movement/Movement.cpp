#include "../GamePlay/Movement/Movement.hpp"
#include "components/Physics2D.hpp"
#include "../Player/PlayerController.hpp"
#include "ecs/Zipper.hpp"


void player_movement_system(Registry& registry, float deltaTime, float speed) {
    auto& transforms = registry.get_components<Transform>();
    auto& rigidbodies = registry.get_components<RigidBody>();
    auto& controllers = registry.get_components<PlayerController>();
    auto& colliders = registry.get_components<BoxCollider>();

    for (auto&& [transform, rigidbody, controller] : Zipper(transforms,
    rigidbodies, controllers)) {
        rigidbody.velocity = {0, 0};  // vitesse controlée par le player
        Vector2 oldPos = transform.position;
        Vector2 direction = controller.get_input_direction();
        rigidbody.velocity += direction * speed;
        transform.position += rigidbody.velocity * deltaTime;
        // nouvelle position=ancienne position+vitesse×Δt
    }
}
