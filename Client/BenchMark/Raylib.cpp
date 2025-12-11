#include <iostream>
extern "C" {
    #include <raylib.h>
}

int main() {
    InitWindow(800, 600, "Raylib Ship Collision (C++)");
  

    Rectangle ship = {375, 500, 50, 50};
    Rectangle obstacle = {350, 300, 100, 50};
    float speed = 300.0f;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        // Movement
        if (IsKeyDown(KEY_LEFT))  ship.x -= speed * delta;
        if (IsKeyDown(KEY_RIGHT)) ship.x += speed * delta;
        if (IsKeyDown(KEY_UP))    ship.y -= speed * delta;
        if (IsKeyDown(KEY_DOWN))  ship.y += speed * delta;

        bool collision = CheckCollisionRecs(ship, obstacle);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(obstacle, RED);
            DrawRectangleRec(ship, collision ? YELLOW : GREEN);
            DrawFPS(10, 10);  // Also show on screen
        EndDrawing();

    }
        // Print FPS to terminal
        //float fps = GetFPS();
        //std::cout << "FPS: " << fps << std::endl;

    CloseWindow();
    return 0;
}
