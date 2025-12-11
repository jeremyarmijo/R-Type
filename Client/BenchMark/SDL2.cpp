#include <SDL2/SDL.h>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL2 Ship Collision",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Renderer sans V-Sync pour FPS réel
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Rect ship = {375, 500, 50, 50};
    SDL_Rect obstacle = {350, 300, 100, 50};
    float speed = 300.0f;

    Uint32 lastTime = SDL_GetTicks();
    Uint32 fpsTimer = SDL_GetTicks();
    int frames = 0;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        Uint32 currentTime = SDL_GetTicks();
        float delta = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT])  ship.x -= speed * delta;
        if (keys[SDL_SCANCODE_RIGHT]) ship.x += speed * delta;
        if (keys[SDL_SCANCODE_UP])    ship.y -= speed * delta;
        if (keys[SDL_SCANCODE_DOWN])  ship.y += speed * delta;

        bool collision = SDL_HasIntersection(&ship, &obstacle);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer,
                               collision ? 255 : 0,
                               collision ? 255 : 255,
                               0, 255);
        SDL_RenderFillRect(renderer, &ship);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &obstacle);

        SDL_RenderPresent(renderer);

        // FPS réel
        frames++;
        if (SDL_GetTicks() - fpsTimer >= 1000) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            fpsTimer = SDL_GetTicks();
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
