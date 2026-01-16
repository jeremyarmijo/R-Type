#pragma once
#include <SDL2/SDL_mixer.h>

#include <string>
#include <unordered_map>

#include "engine/ISubsystem.hpp"
#include "physics/Physics2D.hpp"

class AudioSubsystem : public ISubsystem {
private:
    std::unordered_map<std::string, Mix_Music*> m_music;
    std::unordered_map<std::string, Mix_Chunk*> m_sounds;
    
    std::string m_currentMusic;
    int m_musicVolume;
    int m_sfxVolume;
    bool m_musicMuted;
    bool m_sfxMuted;

public:
    AudioSubsystem();
    ~AudioSubsystem() override;
    
    bool Initialize() override;
    void Shutdown() override;
    void Update(float deltaTime) override;
    void SetRegistry(Registry* registry) override {}
    void ProcessEvent(SDL_Event event) override {}
    
    const char* GetName() const override { return "Audio"; }
    SubsystemType GetType() const override { return SubsystemType::AUDIO; }
    const char* GetVersion() const override { return "1.0.0"; }

    bool LoadMusic(const std::string& name, const std::string& filepath);
    void PlayMusic(const std::string& name, int loops = -1);
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();

    bool LoadSound(const std::string& name, const std::string& filepath);
    void PlaySound(const std::string& name, int loops = 0, int channel = -1);
    void PlaySoundAtPosition(const std::string& name, Vector2 position, int loops = 0);
    void StopSound(int channel = -1);

    void SetMusicVolume(int volume);
    void SetSFXVolume(int volume);
    void MuteMusic(bool mute);
    void MuteSFX(bool mute);

private:
    void UnloadAllAudio();
};

extern "C" {
    ISubsystem* CreateSubsystem();
}
