#pragma once

#include <string>

class Audio {
public:
    Audio() = default;
    ~Audio() = default;

    bool Initialize();
    void Shutdown();
    
    // Sound effects
    void PlaySound(const std::string& filename);
    void SetSoundVolume(float volume);
    
    // Music
    void PlayMusic(const std::string& filename);
    void StopMusic();
    void SetMusicVolume(float volume);

private:
    bool m_initialized = false;
    float m_soundVolume = 1.0f;
    float m_musicVolume = 1.0f;
}; 