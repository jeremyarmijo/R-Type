#include "audio/AudioSubsystem.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

AudioSubsystem::AudioSubsystem()
    : m_currentMusic(""),
      m_musicVolume(64),
      m_sfxVolume(96),
      m_musicMuted(false),
      m_sfxMuted(false) {}

AudioSubsystem::~AudioSubsystem() {
    Shutdown();
}

bool AudioSubsystem::Initialize() {
    std::cout << "Initializing Audio Subsystem..." << std::endl;
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        return false;
    }
    
    Mix_AllocateChannels(16);
    std::cout << "Audio subsystem initialized (16 channels)" << std::endl;
    return true;
}

void AudioSubsystem::Shutdown() {
    std::cout << "Shutting down Audio Subsystem..." << std::endl;
    UnloadAllAudio();
    Mix_CloseAudio();
}

void AudioSubsystem::Update(float deltaTime) {
}

bool AudioSubsystem::LoadMusic(const std::string& name, const std::string& filepath) {
    if (m_music.find(name) != m_music.end()) {
        return true;
    }
    Mix_Music* music = Mix_LoadMUS(filepath.c_str());
if (!music) {
    std::cerr << "Failed to load music: " << filepath << " - " << Mix_GetError() << std::endl;
    return false;
}

m_music[name] = music;
std::cout << "Loaded music: " << name << std::endl;
return true;
}
void AudioSubsystem::PlayMusic(const std::string& name, int loops) {
if (m_musicMuted) return;
auto it = m_music.find(name);
if (it == m_music.end()) {
    std::cerr << "Music not found: " << name << std::endl;
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
}
void AudioSubsystem::StopMusic() {
Mix_HaltMusic();
m_currentMusic = "";
}
void AudioSubsystem::PauseMusic() {
Mix_PauseMusic();
}
void AudioSubsystem::ResumeMusic() {
if (!m_musicMuted) {
Mix_ResumeMusic();
}
}
bool AudioSubsystem::LoadSound(const std::string& name, const std::string& filepath) {
if (m_sounds.find(name) != m_sounds.end()) {
return true;
}
Mix_Chunk* sound = Mix_LoadWAV(filepath.c_str());
if (!sound) {
    std::cerr << "Failed to load sound: " << filepath << " - " << Mix_GetError() << std::endl;
    return false;
}

m_sounds[name] = sound;
std::cout << "Loaded sound: " << name << std::endl;
return true;
}
void AudioSubsystem::PlaySound(const std::string& name, int loops, int channel) {
if (m_sfxMuted) return;
auto it = m_sounds.find(name);
if (it == m_sounds.end()) {
    std::cerr << "Sound not found: " << name << std::endl;
    return;
}

int playedChannel = Mix_PlayChannel(channel, it->second, loops);
if (playedChannel == -1) {
    std::cerr << "Failed to play sound: " << Mix_GetError() << std::endl;
    return;
}

Mix_Volume(playedChannel, m_sfxVolume);
}

void AudioSubsystem::StopSound(int channel) {
Mix_HaltChannel(channel);
}
void AudioSubsystem::SetMusicVolume(int volume) {
m_musicVolume = std::clamp(volume, 0, 128);
Mix_VolumeMusic(m_musicVolume);
}
void AudioSubsystem::SetSFXVolume(int volume) {
m_sfxVolume = std::clamp(volume, 0, 128);
Mix_Volume(-1, m_sfxVolume);
}
void AudioSubsystem::MuteMusic(bool mute) {
m_musicMuted = mute;
Mix_VolumeMusic(mute ? 0 : m_musicVolume);
}
void AudioSubsystem::MuteSFX(bool mute) {
m_sfxMuted = mute;
Mix_Volume(-1, mute ? 0 : m_sfxVolume);
}
void AudioSubsystem::UnloadAllAudio() {
for (auto& [name, music] : m_music) {
Mix_FreeMusic(music);
}
m_music.clear();
for (auto& [name, sound] : m_sounds) {
    Mix_FreeChunk(sound);
}
m_sounds.clear();
}
extern "C" {
ISubsystem* CreateSubsystem() {
return new AudioSubsystem();
}
}