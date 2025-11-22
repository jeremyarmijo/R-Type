#include <iostream>

#include "engine/GameEngine.hpp"

/**
 * @brief example setup to run engine and create various objects such as
 * textures, animations, entities
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char* argv[]) {
  std::cout << "Starting R-Type" << std::endl;

  GameEngine engine;

  if (!engine.Initialize("R-Type", 800, 600)) {
    std::cerr << "Failed to initialize engine!" << std::endl;
    return -1;
  }

  std::cout << "Loading resources" << std::endl;

  TextureManager& textures = engine.GetTextureManager();
  AnimationManager& animations = engine.GetAnimationManager();

  textures.LoadTexture("player", "../assets/player.png");

  animations.CreateAnimation("blue_player", "player",
                             {{{0, 0, 32, 17}, 0.3f},
                              {{32, 0, 32, 17}, 0.3f},
                              {{64, 0, 32, 17}, 0.3f},
                              {{96, 0, 32, 17}, 0.3f},
                              {{128, 0, 32, 17}, 0.3f},
                              {{96, 0, 32, 17}, 0.3f},
                              {{64, 0, 32, 17}, 0.3f},
                              {{32, 0, 32, 17}, 0.3f}},
                             true);

  animations.CreateAnimation("pink_player", "player",
                             {{{0, 17, 32, 17}, 0.3f},
                              {{32, 17, 32, 17}, 0.3f},
                              {{64, 17, 32, 17}, 0.3f},
                              {{96, 17, 32, 17}, 0.3f},
                              {{128, 17, 32, 17}, 0.3f},
                              {{96, 17, 32, 17}, 0.3f},
                              {{64, 17, 32, 17}, 0.3f},
                              {{32, 17, 32, 17}, 0.3f}},
                             true);

  std::cout << "Creating entities" << std::endl;

  Entity player =
      engine.CreatePlayer("player", "blue_player", {200, 300}, 250.0f);

  // Scale up the player
  auto& playerTransform =
      engine.GetRegistry().get_components<Transform>()[player];
  if (playerTransform) {
    playerTransform->scale = {2.0f, 2.0f};
  }

  // Create other entity
  Entity entity =
      engine.CreateAnimatedSprite("player", {300.0f, 200.0f}, "pink_player");
  // add physics
  engine.GetRegistry().add_component(entity, RigidBody{1.0f, 0.3f, false});
  engine.GetRegistry().add_component(entity, BoxCollider{60, 32});
  auto& entityTransform =
      engine.GetRegistry().get_components<Transform>()[entity];
  if (entityTransform) {
    entityTransform->scale = {2.0f, 2.0f};
  }

  engine.Run();

  engine.Shutdown();

  return 0;
}
