#pragma once

#include <unordered_map>

// Forward declaration to avoid including SDL in header
union SDL_Event;

class Input {
public:
    Input() = default;
    ~Input() = default;

    bool Initialize();
    void Shutdown();
    void Update();
    
    void HandleEvent(const SDL_Event& event);
    
    // Mouse input
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonDown(int button) const;
    void GetMousePosition(int& x, int& y) const;
    
    // Keyboard input
    bool IsKeyPressed(int key) const;
    bool IsKeyDown(int key) const;

private:
    std::unordered_map<int, bool> m_mouseButtons;
    std::unordered_map<int, bool> m_mouseButtonsPrev;
    std::unordered_map<int, bool> m_keys;
    std::unordered_map<int, bool> m_keysPrev;
    
    int m_mouseX = 0;
    int m_mouseY = 0;
}; 