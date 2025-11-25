#pragma once
#include <SDL2/SDL.h>

#include <string>

#include "physics/Physics2D.hpp"

struct Sprite {
  std::string textureKey;
  SDL_Rect sourceRect;
  Vector2 pivot;  // 0,0 = top left, 0.5,0.5 = center
  int layer;

  Sprite(const std::string& key, SDL_Rect src = {0, 0, 0, 0},
         Vector2 piv = {0.5f, 0.5f}, int lyr = 0)
      : textureKey(key), sourceRect(src), pivot(piv), layer(lyr) {}
};

struct Animation {
  std::string currentAnimation;
  float currentTime;
  int currentFrame;
  bool isPlaying;

  explicit Animation(const std::string& anim = "", bool playing = false)
      : currentAnimation(anim),
        currentTime(0),
        currentFrame(0),
        isPlaying(playing) {}
};

struct Camera {
  Vector2 position;
  float zoom;
  bool isActive;

  explicit Camera(Vector2 pos = {0, 0}, float z = 1.0f, bool active = true)
      : position(pos), zoom(z), isActive(active) {}
};
