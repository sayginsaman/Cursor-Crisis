#include "Audio.h"
#include <iostream>

bool Audio::Initialize() {
    std::cout << "Audio system initialized" << std::endl;
    m_initialized = true;
    return true;
}

void Audio::Shutdown() {
    m_initialized = false;
    std::cout << "Audio system shutdown" << std::endl;
}

void Audio::PlaySound(const std::string& filename) {
    if (!m_initialized) return;
    std::cout << "Playing sound: " << filename << std::endl;
}

void Audio::SetSoundVolume(float volume) {
    m_soundVolume = volume;
    std::cout << "Sound volume set to: " << volume << std::endl;
}

void Audio::PlayMusic(const std::string& filename) {
    if (!m_initialized) return;
    std::cout << "Playing music: " << filename << std::endl;
}

void Audio::StopMusic() {
    if (!m_initialized) return;
    std::cout << "Music stopped" << std::endl;
}

void Audio::SetMusicVolume(float volume) {
    m_musicVolume = volume;
    std::cout << "Music volume set to: " << volume << std::endl;
} 