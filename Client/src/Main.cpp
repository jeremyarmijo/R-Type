#include <iostream>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/SceneManager.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIImage.hpp"
#include "ui/UIText.hpp"

class MyGameScene : public Scene {
 private:
  std::vector<Entity> m_entities;
  int m_score;
  bool m_isInitialized;

 public:
  MyGameScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "game"),
        m_score(0),
        m_isInitialized(false) {}

  void OnEnter() override {
    std::cout << "\n=== ENTERING GAME SCENE ===" << std::endl;

    try {
      // Clear old data
      m_entities.clear();
      m_score = 0;
      m_isInitialized = false;

      TextureManager& textures = GetTextures();
      AnimationManager& animations = GetAnimations();

      std::cout << "Loading textures..." << std::endl;
      if (!textures.GetTexture("player")) {
        textures.LoadTexture("player", "../Client/assets/player.png");
      }
      if (!textures.GetTexture("background")) {
        textures.LoadTexture("background", "../Client/assets/bg.jpg");
      }

      std::cout << "Creating animations..." << std::endl;
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

      std::cout << "Creating background..." << std::endl;
      Entity background = m_engine->CreateSprite("background", {400, 300}, -10);
      m_entities.push_back(background);
      std::cout << "Creating player..." << std::endl;
      Entity player =
          m_engine->CreatePlayer("player", "blue_player", {200, 300}, 250.0f);
      m_entities.push_back(player);

      if (!GetRegistry().is_entity_valid(player)) {
        std::cerr << "ERROR: Player entity is invalid after creation!"
                  << std::endl;
        return;
      }

      auto* scoreText = GetUI().AddElement<UIText>(10, 10, "Score: 0");
      scoreText->SetLayer(100);  // UI on top

      auto* healthBar =
          GetUI().AddElement<UIImage>(10, 50, 200, 20, "health_bar");
      healthBar->SetLayer(100);

      m_isInitialized = true;
      std::cout << "Game scene initialized successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }

    std::cout << "=================================\n" << std::endl;
    DebugComponents();
  }

  void DebugComponents() {
    auto& animations = GetRegistry().get_components<Animation>();
    auto& sprites = GetRegistry().get_components<Sprite>();

    std::cout << "\n=== COMPONENT DEBUG ===" << std::endl;

    for (size_t i = 0; i < 4; ++i) {
      std::cout << "Entity " << i << ": ";

      bool hasAnim = (i < animations.size() && animations[i].has_value());
      bool hasSprite = (i < sprites.size() && sprites[i].has_value());

      std::cout << "Anim=" << (hasAnim ? "YES" : "NO") << " ";
      std::cout << "Sprite=" << (hasSprite ? "YES" : "NO");

      if (hasAnim) {
        std::cout << " [" << animations[i]->currentAnimation << "]";
      }

      std::cout << std::endl;
    }
    std::cout << "====================\n" << std::endl;
  }

  void OnExit() override {
    std::cout << "\n=== EXITING GAME SCENE ===" << std::endl;

    m_entities.clear();
    m_isInitialized = false;

    std::cout << "Game scene cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) {
      return;
    }

    auto& network = GetNetwork();

    network.SendAction();
  }

  void Render() override {
    if (!m_isInitialized) return;

    RenderSpritesLayered();
    GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (GetUI().HandleEvent(event)) {
      return;
    }

    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == SDLK_r) {
        std::cout << "Restarting level..." << std::endl;
        ChangeScene("game");
      } else if (event.key.keysym.sym == SDLK_x) {
        std::cout << "Quitting to Game Over screen..." << std::endl;
        ChangeScene("gameover");
      }
    }
  }
};

class GameOverScene : public Scene {
 private:
  int m_finalScore;
  bool m_isInitialized;

 public:
  GameOverScene(GameEngine* engine, SceneManager* sceneManager)
      : Scene(engine, sceneManager, "GameOver"),
        m_finalScore(0),
        m_isInitialized(false) {}

  void SetScore(int score) { m_finalScore = score; }

  void OnEnter() override {
    std::cout << "\n=== GAME OVER ===" << std::endl;

    // could create game over UI entities here
    m_isInitialized = true;
  }

  void OnExit() override {
    std::cout << "Leaving Game Over screen..." << std::endl;
    m_isInitialized = false;
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
    std::cout << "=== INITIALIZING GAME ===" << std::endl;

    if (!m_engine.Initialize("R-Type Game", 800, 600)) {
      std::cerr << "ERROR: Failed to initialize engine!" << std::endl;
      return false;
    }

    // CRITICAL: Connect scene manager to engine FIRST
    m_engine.SetSceneManager(&m_sceneManager);

    // CRITICAL: Give scene manager access to registry for cleanup
    m_sceneManager.SetRegistry(&m_engine.GetRegistry());

    std::cout << "Registering scenes..." << std::endl;
    m_sceneManager.RegisterScene<MyGameScene>("game", &m_engine,
                                              &m_sceneManager);
    m_sceneManager.RegisterScene<GameOverScene>("gameover", &m_engine,
                                                &m_sceneManager);

    std::cout << "Starting initial scene..." << std::endl;
    m_sceneManager.ChangeScene("game");

    std::cout << "Initialization complete!\n" << std::endl;
    return true;
  }

  void Run() {
    // Engine calls scene.Update() and scene.Render() automatically
    m_engine.Run();
  }

  void Shutdown() {
    std::cout << "=== SHUTTING DOWN ===" << std::endl;
    m_engine.Shutdown();
  }
};

int main(int argc, char* argv[]) {
  MyGame game;

  if (!game.Initialize()) {
    std::cerr << "Failed to initialize!" << std::endl;
    return -1;
  }

  try {
    game.Run();
  } catch (const std::exception& e) {
    std::cerr << "FATAL ERROR: " << e.what() << std::endl;
    return -1;
  }
  game.Shutdown();

  return 0;
}
