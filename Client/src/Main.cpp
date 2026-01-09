#include <iostream>
#include <vector>

#include "engine/GameEngine.hpp"
#include "scene/SceneManager.hpp"
#include "scenes/CreateLobby.hpp"
#include "scenes/GameOver.hpp"
#include "scenes/JoinGame.hpp"
#include "scenes/Lobby.hpp"
#include "scenes/LobbyInfo.hpp"
#include "scenes/LobbyJoin.hpp"
#include "scenes/MainGame.hpp"
#include "scenes/MainMenu.hpp"
#include "scenes/Options.hpp"
#include "scenes/WaitLobby.hpp"
#include "scenes/LobbyPassword.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIImage.hpp"
#include "ui/UIText.hpp"

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

    m_engine.SetSceneManager(&m_sceneManager);
    m_sceneManager.SetRegistry(&m_engine.GetRegistry());

    std::cout << "Registering scenes..." << std::endl;
    m_sceneManager.RegisterScene<JoinGame>("join", &m_engine, &m_sceneManager);
    m_sceneManager.RegisterScene<MainMenu>("menu", &m_engine, &m_sceneManager);
    m_sceneManager.RegisterScene<OptionsScene>("options", &m_engine,
                                               &m_sceneManager);
    m_sceneManager.RegisterScene<MyGameScene>("game", &m_engine,
                                              &m_sceneManager);
    m_sceneManager.RegisterScene<GameOverScene>("gameover", &m_engine,
                                                &m_sceneManager);
    m_sceneManager.RegisterScene<WaitLobby>("wait", &m_engine, &m_sceneManager);
    m_sceneManager.RegisterScene<LobbyMenu>("lobby", &m_engine,
                                            &m_sceneManager);
    m_sceneManager.RegisterScene<CreateLobby>("createLobby", &m_engine,
                                              &m_sceneManager);
    m_sceneManager.RegisterScene<LobbyInfoPlayer>("lobbyInfoPlayer", &m_engine,
                                                  &m_sceneManager);
    m_sceneManager.RegisterScene<LobbyJoin>("lobbyjoin", &m_engine,
                                            &m_sceneManager);
    m_sceneManager.RegisterScene<LobbyPassword>("lobbyPassword", &m_engine,
                                            &m_sceneManager);

    std::cout << "Starting initial scene..." << std::endl;
    m_sceneManager.ChangeScene("menu");

    std::cout << "Initialization complete!\n" << std::endl;
    return true;
  }

  void Run() { m_engine.Run(); }

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
