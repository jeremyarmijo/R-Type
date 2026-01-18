#pragma once
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/ISubsystem.hpp"
#include "ecs/Registry.hpp"
#include "physics/Physics2D.hpp"
#include "rendering/rendering_export.hpp"

struct AnimationFrame {
    SDL_Rect sourceRect;
    float duration;
};

struct AnimationClip {
    std::string textureKey;
    std::vector<AnimationFrame> frames;
    bool loop;
    
    explicit AnimationClip(bool shouldLoop = true) : loop(shouldLoop) {}
};

struct Sprite {
    std::string textureKey;
    SDL_Rect sourceRect;
    Vector2 pivot;
    int layer;
    bool visible;
    
    Sprite(const std::string& key = "", SDL_Rect src = {0,0,0,0},
           Vector2 piv = {0.5f, 0.5f}, int lay = 0)
        : textureKey(key), sourceRect(src), pivot(piv), layer(lay), visible(true) {}
};

struct Animation {
    std::string animationKey;
    int currentFrame;
    float currentTime;
    bool isPlaying;
    bool loop;
    
    Animation(const std::string& key = "", bool shouldLoop = true)
        : animationKey(key), currentFrame(0), currentTime(0.0f),
          isPlaying(false), loop(shouldLoop) {}
};

struct Camera {
    Vector2 position;
    Vector2 offset;
    float zoom;
    
    Camera() : position{0, 0}, offset{0, 0}, zoom(1.0f) {}
};

class UIElement;
class UIManager;

class RENDERING_API RenderingSubsystem : public ISubsystem {
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    
    std::unordered_map<std::string, SDL_Texture*> m_textures;
    
    std::unordered_map<std::string, AnimationClip> m_animations;
    
    std::unique_ptr<UIManager> m_uiManager;
    
    Vector2 m_cameraPosition;
    Registry* m_registry;
    
    int m_windowWidth;
    int m_windowHeight;

public:
    RenderingSubsystem();
    ~RenderingSubsystem() override;
    
    bool Initialize() override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    void ProcessEvent(SDL_Event event) override {}
    
    const char* GetName() const override { return "Rendering"; }
    SubsystemType GetType() const override { return SubsystemType::RENDERING; }
    const char* GetVersion() const override { return "1.0.0"; }
    
    void SetRegistry(Registry* registry) override;
    void SetWindowSize(int width, int height);
    void SetWindowTitle(const std::string& title);
    void SetCameraPosition(Vector2 pos) { m_cameraPosition = pos; }
    Vector2 GetCameraPosition() const { return m_cameraPosition; }
    
    SDL_Renderer* GetRenderer() { return m_renderer; }
    
    bool LoadTexture(const std::string& key, const std::string& filepath);
    SDL_Texture* GetTexture(const std::string& key);
    void UnloadTexture(const std::string& key);
    void GetTextureSize(const std::string& key, int& width, int& height);
    
    void CreateAnimation(const std::string& name, const std::string& textureKey,
                        const std::vector<AnimationFrame>& frames, bool loop = true);
    const AnimationClip* GetAnimation(const std::string& name) const;
    void RemoveAnimation(const std::string& name);
    
    UIManager* GetUIManager() { return m_uiManager.get(); }

private:
    void RenderSprites();
    void UpdateAnimations(float deltaTime);
};

#ifdef _WIN32
extern "C" {
__declspec(dllexport) ISubsystem* CreateSubsystem();
}
#else
extern "C" {
    ISubsystem* CreateSubsystem();
}
#endif