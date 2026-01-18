#include <iostream>
#include <vector>

#include "dynamicLibLoader/DLLoader.hpp"
#include "engine/GameEngine.hpp"
#include "engine/ISubsystem.hpp"
#include "physics/PhysicsSystem.hpp"
#include "rendering/RenderingSubsystem.hpp"

int main(int argc, char* argv[]) {
  try {
    GameEngine engine;

    if (!engine.Initialize("R-Type Game", 800, 600)) {
      std::cerr << "Failed to initialize engine!" << std::endl;
      return -1;
    }

    std::cout << "\n=== LOADING SUBSYSTEMS ===" << std::endl;

    engine.LoadSubsystem(SubsystemType::RENDERING,
                         "../../EngineModule/build/libsubsystem_rendering.so");
    engine.LoadSubsystem(SubsystemType::AUDIO,
                         "../../EngineModule/build/libsubsystem_audio.so");
    engine.LoadSubsystem(SubsystemType::INPUT,
                         "../../EngineModule/build/libsubsystem_input.so");
    engine.LoadSubsystem(SubsystemType::PHYSICS,
                         "../../EngineModule/build/libsubsystem_physics.so");
    engine.LoadSubsystem(SubsystemType::MESSAGING,
                         "../../EngineModule/build/libsubsystem_messaging.so");
    engine.LoadSubsystem(SubsystemType::NETWORK,
                         "../../EngineModule/build/libsubsystem_network.so");

    std::cout << "\n=== LOADING GAME SCENES ===" << std::endl;

    engine.GetSceneManager().LoadSceneModule(
        "createLobby", "../src/scenes/libscene_createlobby.so");
    engine.GetSceneManager().LoadSceneModule(
        "gameover", "../src/scenes/libscene_gameover.so");
    engine.GetSceneManager().LoadSceneModule(
        "join", "../src/scenes/libscene_joingame.so");
    engine.GetSceneManager().LoadSceneModule("lobby",
                                             "../src/scenes/libscene_lobby.so");
    engine.GetSceneManager().LoadSceneModule(
        "lobbyInfoPlayer", "../src/scenes/libscene_lobbyinfo.so");
    engine.GetSceneManager().LoadSceneModule(
        "lobbyjoin", "../src/scenes/libscene_lobbyjoin.so");
    engine.GetSceneManager().LoadSceneModule(
        "lobbyPassword", "../src/scenes/libscene_lobbypassword.so");
    engine.GetSceneManager().LoadSceneModule(
        "game", "../src/scenes/libscene_maingame.so");
    engine.GetSceneManager().LoadSceneModule(
        "menu", "../src/scenes/libscene_mainmenu.so");
    engine.GetSceneManager().LoadSceneModule(
        "options", "../src/scenes/libscene_options.so");
    engine.GetSceneManager().LoadSceneModule(
        "wait", "../src/scenes/libscene_waitlobby.so");

    std::cout << "\n=== STARTING GAME ===" << std::endl;

    engine.ChangeScene("menu");

    engine.Run();

    std::cout << "\n=== GAME ENDED ===" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "FATAL ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
