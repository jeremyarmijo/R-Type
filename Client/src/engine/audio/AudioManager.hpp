#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <iostream>

class AudioManager {
private:
    std::unordered_map<std::string, Mix_Music*> m_music;
    std::unordered_map<std::string, Mix_Chunk*> m_sounds;
    std::string m_currentMusic;

    int m_musicVolume;
    int m_sfxVolume;

    bool m_musicMuted;
    bool m_sfxMuted;

public:
    AudioManager()
        : m_currentMusic(""),
          m_musicVolume(64),
          m_sfxVolume(96),
          m_musicMuted(false),
          m_sfxMuted(false) {}

    ~AudioManager() {
        Cleanup();
    }

    bool Initialize(int frequency = 44100, Uint16 format = MIX_DEFAULT_FORMAT, 
                   int channels = 2, int chunksize = 2048) {
        if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
            std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " 
                      << Mix_GetError() << std::endl;
            return false;
        }

        Mix_AllocateChannels(16);

        std::cout << "AudioManager initialized successfully" << std::endl;
        return true;
    }

    bool LoadMusic(const std::string& name, const std::string& filepath) {
        if (m_music.find(name) != m_music.end()) {
            std::cout << "Music '" << name << "' already loaded" << std::endl;
            return true;
        }

        Mix_Music* music = Mix_LoadMUS(filepath.c_str());
        if (!music) {
            std::cerr << "Failed to load music '" << filepath << "': " 
                      << Mix_GetError() << std::endl;
            return false;
        }

        m_music[name] = music;
        std::cout << "Loaded music: " << name << " from " << filepath << std::endl;
        return true;
    }

    bool LoadSound(const std::string& name, const std::string& filepath) {
        if (m_sounds.find(name) != m_sounds.end()) {
            std::cout << "Sound '" << name << "' already loaded" << std::endl;
            return true;
        }

        Mix_Chunk* sound = Mix_LoadWAV(filepath.c_str());
        if (!sound) {
            std::cerr << "Failed to load sound '" << filepath << "': " 
                      << Mix_GetError() << std::endl;
            return false;
        }

        m_sounds[name] = sound;
        std::cout << "Loaded sound: " << name << " from " << filepath << std::endl;
        return true;
    }

    void PlayMusic(const std::string& name, int loops = -1) {
        if (m_musicMuted) return;

        auto it = m_music.find(name);
        if (it == m_music.end()) {
            std::cerr << "Music '" << name << "' not found!" << std::endl;
            return;
        }

        if (m_currentMusic != name) {
            StopMusic();
        }

        if (Mix_PlayMusic(it->second, loops) == -1) {
            std::cerr << "Failed to play music: " << Mix_GetError() << std::endl;
            return;
        }

        m_currentMusic = name;
        Mix_VolumeMusic(m_musicVolume);
        std::cout << "Playing music: " << name << std::endl;
    }

    void StopMusic() {
        Mix_HaltMusic();
        m_currentMusic = "";
    }

    void PauseMusic() {
        Mix_PauseMusic();
    }

    void ResumeMusic() {
        if (!m_musicMuted) {
            Mix_ResumeMusic();
        }
    }

    bool IsMusicPlaying() const {
        return Mix_PlayingMusic() != 0;
    }

    void PlaySound(const std::string& name, int loops = 0, int channel = -1) {
        if (m_sfxMuted) return;

        auto it = m_sounds.find(name);
        if (it == m_sounds.end()) {
            std::cerr << "Sound '" << name << "' not found!" << std::endl;
            return;
        }

        int playedChannel = Mix_PlayChannel(channel, it->second, loops);
        if (playedChannel == -1) {
            std::cerr << "Failed to play sound: " << Mix_GetError() << std::endl;
            return;
        }

        Mix_Volume(playedChannel, m_sfxVolume);
    }

    void StopSound(int channel = -1) {
        Mix_HaltChannel(channel);
    }

    void SetMusicVolume(int volume) {
        m_musicVolume = std::clamp(volume, 0, 128);
        Mix_VolumeMusic(m_musicVolume);
    }

    void SetSFXVolume(int volume) {
        m_sfxVolume = std::clamp(volume, 0, 128);
        Mix_Volume(-1, m_sfxVolume);
    }

    int GetMusicVolume() const { return m_musicVolume; }
    int GetSFXVolume() const { return m_sfxVolume; }

    void MuteMusic(bool mute) {
        m_musicMuted = mute;
        if (mute) {
            Mix_VolumeMusic(0);
        } else {
            Mix_VolumeMusic(m_musicVolume);
        }
    }

    void MuteSFX(bool mute) {
        m_sfxMuted = mute;
        if (mute) {
            Mix_Volume(-1, 0);
        } else {
            Mix_Volume(-1, m_sfxVolume);
        }
    }

    bool IsMusicMuted() const { return m_musicMuted; }
    bool IsSFXMuted() const { return m_sfxMuted; }

    void Cleanup() {
        for (auto& [name, music] : m_music) {
            if (music) {
                Mix_FreeMusic(music);
            }
        }
        m_music.clear();

        for (auto& [name, sound] : m_sounds) {
            if (sound) {
                Mix_FreeChunk(sound);
            }
        }
        m_sounds.clear();

        std::cout << "AudioManager cleaned up" << std::endl;
    }

    void Shutdown() {
        Cleanup();
        Mix_CloseAudio();
        std::cout << "AudioManager shutdown complete" << std::endl;
    }

    void PrintDebug() const {
        std::cout << "\n=== Audio Manager Debug ===" << std::endl;
        std::cout << "Music tracks loaded: " << m_music.size() << std::endl;
        std::cout << "Sound effects loaded: " << m_sounds.size() << std::endl;
        std::cout << "Current music: " << (m_currentMusic.empty() ? "None" : m_currentMusic) << std::endl;
        std::cout << "Music volume: " << m_musicVolume << "/128 " 
                  << (m_musicMuted ? "(MUTED)" : "") << std::endl;
        std::cout << "SFX volume: " << m_sfxVolume << "/128 " 
                  << (m_sfxMuted ? "(MUTED)" : "") << std::endl;
        std::cout << "=========================\n" << std::endl;
    }
};
