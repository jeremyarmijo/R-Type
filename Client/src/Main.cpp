#include <iostream>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/SceneManager.hpp"

class MyGameScene : public Scene {
 private:
  std::vector<Entity> m_players;
  std::vector<Entity> m_enemies;
  int m_score;
  float m_gameTime;

 public:
  MyGameScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "game"), m_score(0), m_gameTime(0.0f) {}

  void OnEnter() override {
    std::cout << "=== Level Start ===" << std::endl;

    TextureManager& textures = GetTextures();
    AnimationManager& animations = GetAnimations();

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

    // Create player
    Entity player =
        m_engine->CreatePlayer("player", "blue_player", {200, 300}, 250.0f);
    m_players.push_back(player);

    // Create enemies
    for (int i = 0; i < 3; ++i) {
      Entity enemy = m_engine->CreateAnimatedSprite(
          "player", {400.0f + i * 150.0f, 200.0f}, "pink_player");
      GetRegistry().add_component(enemy, RigidBody{1.0f, 0.3f, false});
      GetRegistry().add_component(enemy, BoxCollider{32, 32});
      m_enemies.push_back(enemy);
    }

    // Reset game state
    m_score = 0;
    m_gameTime = 0.0f;
  }

  void OnExit() override {
    std::cout << "=== Level End ===" << std::endl;
    std::cout << "Final Score: " << m_score << std::endl;

    // Clean up entities
    for (auto player : m_players) {
      GetRegistry().kill_entity(player);
    }
    for (auto enemy : m_enemies) {
      GetRegistry().kill_entity(enemy);
    }

    m_players.clear();
    m_enemies.clear();
  }

  void Update(float deltaTime) override {
    m_gameTime += deltaTime;

    // Check win condition
    if (m_enemies.empty()) {
      std::cout << "YOU WIN! Score: " << m_score << std::endl;
      std::cout << "Going to Game Over screen..." << std::endl;

      ChangeScene("gameover");
      return;
    }
  }

  void Render() override {
    // Render score/UI here if needed
  }

  void HandleEvent(SDL_Event& event) override {
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_r) {
        std::cout << "Restarting level..." << std::endl;
        OnExit();
        OnEnter();
      } else if (event.key.keysym.sym == SDLK_ESCAPE) {
        std::cout << "Quitting to Game Over screen..." << std::endl;
        ChangeScene("gameover");
      }
    }
  }
};

class GameOverScene : public Scene {
 private:
  int m_finalScore;

 public:
  GameOverScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "GameOver"), m_finalScore(0) {}

  void SetScore(int score) { m_finalScore = score; }

  void OnEnter() override {
    std::cout << "\n=== GAME OVER ===" << std::endl;

    // could create game over UI entities here
  }

  void OnExit() override {
    std::cout << "Leaving Game Over screen..." << std::endl;
  }

  void Update(float deltaTime) override {}

  void Render() override {}

  void HandleEvent(SDL_Event& event) override {
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_SPACE ||
          event.key.keysym.sym == SDLK_RETURN) {
        std::cout << "Restarting game..." << std::endl;
        ChangeScene("game");
      }
    }
  }
};

class MyGame {
 private:
  GameEngine m_engine;
  SceneManager m_sceneManager;

 public:
  bool Initialize() {
    if (!m_engine.Initialize("Simple Game", 800, 600)) {
      return false;
    }

    // Connect engine to scene manager
    m_engine.SetSceneManager(&m_sceneManager);

    m_sceneManager.RegisterScene<MyGameScene>("game", &m_engine,
                                              &m_sceneManager);
    m_sceneManager.RegisterScene<GameOverScene>("gameover", &m_engine,
                                                &m_sceneManager);

    // Start game
    m_sceneManager.ChangeScene("game");

    return true;
  }

  void Run() {
    // Engine calls scene.Update() and scene.Render() automatically
    m_engine.Run();
  }

  void Shutdown() { m_engine.Shutdown(); }
};

int main(int argc, char* argv[]) {
  MyGame game;

  if (!game.Initialize()) {
    std::cerr << "Failed to initialize!" << std::endl;
    return -1;
  }

  game.Run();
  game.Shutdown();

  return 0;
}
