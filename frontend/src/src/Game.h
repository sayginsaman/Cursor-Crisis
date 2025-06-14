#pragma once

#include <memory>
#include <stack>
#include <SDL2/SDL.h>
#include <GL/glew.h>

// Forward declarations for SDL types
struct SDL_Window;
typedef void* SDL_GLContext;
typedef unsigned int Uint32;

// Forward declarations
class GameState;
class Renderer;
class Input;
class Audio;

class Game {
public:
    Game();
    ~Game();

    // Core game functions
    bool Initialize();
    void Run();
    void Shutdown();

    // Game state management
    void PushState(std::unique_ptr<GameState> state);
    void PopState();
    void ChangeState(std::unique_ptr<GameState> state);

    // Getters
    SDL_Window* GetWindow() const { return m_window; }
    SDL_GLContext GetGLContext() const { return m_glContext; }
    Renderer* GetRenderer() const { return m_renderer.get(); }
    Input* GetInput() const { return m_input.get(); }
    Audio* GetAudio() const { return m_audio.get(); }
    
    bool IsRunning() const { return m_running; }
    void SetRunning(bool running) { m_running = running; }

    // Game settings
    int GetScreenWidth() const { return m_screenWidth; }
    int GetScreenHeight() const { return m_screenHeight; }
    bool IsFullscreen() const { return m_fullscreen; }
    
    void SetFullscreen(bool fullscreen);
    void SetVSync(bool vsync);

private:
    // SDL and OpenGL
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    
    // Core systems
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<Audio> m_audio;
    
    // Game state stack
    std::stack<std::unique_ptr<GameState>> m_states;
    
    // Game settings
    int m_screenWidth;
    int m_screenHeight;
    bool m_fullscreen;
    bool m_vsync;
    bool m_running;
    
    // Frame timing
    Uint32 m_lastFrameTime;
    float m_deltaTime;
    
    // FPS tracking
    int m_frameCount;
    Uint32 m_fpsTimer;
    float m_fps;

    // Private methods
    bool InitializeSDL();
    bool InitializeOpenGL();
    void CalculateDeltaTime();
    void UpdateFPS();
    void HandleEvents();
    void Update(float deltaTime);
    void Render();
}; 