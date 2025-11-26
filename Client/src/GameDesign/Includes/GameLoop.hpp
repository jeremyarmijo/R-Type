#include "../../engine/GameEngine.hpp"
#include "network/NetworkManager.hpp"
#include <SDL2/SDL.h>

class GameLoop {
    protected:
        GameEngine* engine; 
        NetworkManager* network;
        bool isRunning;
        SDL_Window* window;
        SDL_Renderer* renderer;
    private:
        GameLoop() : isRunning(false), window(nullptr), renderer(nullptr) {}
        bool Initialize();
        void Run();
        void Shutdown();
};