#include "Input.h"
#include <SDL2/SDL.h>
#include <iostream>

bool Input::Initialize() {
    std::cout << "Input system initialized" << std::endl;
    return true;
}

void Input::Shutdown() {
    std::cout << "Input system shutdown" << std::endl;
}

void Input::Update() {
    // Copy current state to previous state
    m_mouseButtonsPrev = m_mouseButtons;
    m_keysPrev = m_keys;
}

void Input::HandleEvent(const SDL_Event& event) {
    // Handle SDL events and update input state
    // This will be implemented once SDL headers are properly included
}

bool Input::IsMouseButtonPressed(int button) const {
    auto current = m_mouseButtons.find(button);
    auto prev = m_mouseButtonsPrev.find(button);
    
    bool currentPressed = (current != m_mouseButtons.end()) ? current->second : false;
    bool prevPressed = (prev != m_mouseButtonsPrev.end()) ? prev->second : false;
    
    return currentPressed && !prevPressed;
}

bool Input::IsMouseButtonDown(int button) const {
    auto it = m_mouseButtons.find(button);
    return (it != m_mouseButtons.end()) ? it->second : false;
}

void Input::GetMousePosition(int& x, int& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

bool Input::IsKeyPressed(int key) const {
    auto current = m_keys.find(key);
    auto prev = m_keysPrev.find(key);
    
    bool currentPressed = (current != m_keys.end()) ? current->second : false;
    bool prevPressed = (prev != m_keysPrev.end()) ? prev->second : false;
    
    return currentPressed && !prevPressed;
}

bool Input::IsKeyDown(int key) const {
    auto it = m_keys.find(key);
    return (it != m_keys.end()) ? it->second : false;
} 