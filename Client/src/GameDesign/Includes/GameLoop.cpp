#include "GameLoop.hpp"
#include <iostream>

bool GameLoop::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_init Error: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error SDL_CreateWindow: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Erreur SDL_CreateRenderer: " << SDL_GetError() << std::endl;
        return false;
    }
    isRunning = true;
    return true;
}

void GameLoop::Run()
{
    SDL_Event event;
        while (isRunning) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    isRunning = false;
                }
            }
            SDL_SetRenderDrawColor(renderer, 50, 50, 80, 255);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
        Shutdown();
}

void GameLoop::Shutdown()
{
     if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    GameLoop game;
    if (!game.Initialize()) 
        return -1;
    game.Run();
    return 0;
}
