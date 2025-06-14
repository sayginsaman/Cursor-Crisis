#include "Game.h"
#include "GameState.h"
#include "HomeState.h"
#include "AuthChoiceState.h"
#include "Renderer.h"
#include "Input.h"
#include "Audio.h"
#include <iostream>
#include <cstring>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

Game::Game() 
    : m_window(nullptr)
    , m_glContext(nullptr)
    , m_screenWidth(1280)
    , m_screenHeight(720)
    , m_fullscreen(false)
    , m_vsync(true)
    , m_running(false)
    , m_lastFrameTime(0)
    , m_deltaTime(0.0f)
    , m_frameCount(0)
    , m_fpsTimer(0)
    , m_fps(0.0f)
{
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    // Initialize SDL
    if (!InitializeSDL()) {
        std::cerr << "Failed to initialize SDL!" << std::endl;
        return false;
    }
    
    // Initialize OpenGL
    if (!InitializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL!" << std::endl;
        return false;
    }
    
    // Core systems
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize()) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return false;
    }
    
    m_input = std::make_unique<Input>();
    if (!m_input->Initialize()) {
        std::cerr << "Failed to initialize input!" << std::endl;
        return false;
    }
    
    m_audio = std::make_unique<Audio>();
    if (!m_audio->Initialize()) {
        std::cerr << "Failed to initialize audio!" << std::endl;
        return false;
    }
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    
    // Check OpenGL version and use appropriate GLSL version
    const char* gl_version = (const char*)glGetString(GL_VERSION);
    std::cout << "Detected OpenGL Version: " << gl_version << std::endl;
    
    // Use appropriate GLSL version based on OpenGL version
    if (strstr(gl_version, "2.1")) {
        std::cout << "Using GLSL 1.20 for OpenGL 2.1" << std::endl;
        ImGui_ImplOpenGL3_Init("#version 120");
    } else {
        std::cout << "Using GLSL 3.30 for OpenGL 3.3+" << std::endl;
        ImGui_ImplOpenGL3_Init("#version 330");
    }
    
    // Push initial state (Authentication Choice for first-time users)
    PushState(std::make_unique<AuthChoiceState>(this));
    
    m_running = true;
    m_lastFrameTime = SDL_GetTicks();
    m_fpsTimer = SDL_GetTicks();
    
    std::cout << "Game initialized successfully!" << std::endl;
    return true;
}

void Game::Run() {
    while (m_running && !m_states.empty()) {
        CalculateDeltaTime();
        UpdateFPS();
        
        HandleEvents();
        Update(m_deltaTime);
        Render();
    }
}

void Game::Shutdown() {
    // Clear all states
    while (!m_states.empty()) {
        m_states.pop();
    }
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup core systems
    m_audio.reset();
    m_input.reset();
    m_renderer.reset();
    
    // Cleanup SDL
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
    
    std::cout << "Game shutdown complete." << std::endl;
}

void Game::PushState(std::unique_ptr<GameState> state) {
    if (!m_states.empty()) {
        m_states.top()->OnPause();
    }
    m_states.push(std::move(state));
    m_states.top()->OnEnter();
}

void Game::PopState() {
    if (!m_states.empty()) {
        m_states.top()->OnExit();
        m_states.pop();
        
        if (!m_states.empty()) {
            m_states.top()->OnResume();
        }
    }
}

void Game::ChangeState(std::unique_ptr<GameState> state) {
    if (!m_states.empty()) {
        m_states.top()->OnExit();
        m_states.pop();
    }
    m_states.push(std::move(state));
    m_states.top()->OnEnter();
}

void Game::SetFullscreen(bool fullscreen) {
    if (m_fullscreen == fullscreen) return;
    
    m_fullscreen = fullscreen;
    
    if (fullscreen) {
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(m_window, 0);
        SDL_SetWindowSize(m_window, m_screenWidth, m_screenHeight);
        // Center window
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}

void Game::SetVSync(bool vsync) {
    m_vsync = vsync;
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

bool Game::InitializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Try different OpenGL configurations
    // First try: OpenGL 3.3 Compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    // Create window
    m_window = SDL_CreateWindow(
        "Desktop Survivor Dash",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_screenWidth,
        m_screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!m_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Try to create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cout << "OpenGL 3.3 Compatibility failed, trying 2.1..." << std::endl;
        
        // Fallback: Try OpenGL 2.1
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0); // No profile for 2.1
        
        m_glContext = SDL_GL_CreateContext(m_window);
        if (!m_glContext) {
            std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
    }
    
    // Enable VSync by default
    SetVSync(m_vsync);
    
    return true;
}

bool Game::InitializeOpenGL() {
    // Initialize GLEW
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
        return false;
    }
    
    // Check OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    // Set viewport
    glViewport(0, 0, m_screenWidth, m_screenHeight);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set clear color (dark blue-gray)
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    
    return true;
}

void Game::CalculateDeltaTime() {
    Uint32 currentTime = SDL_GetTicks();
    m_deltaTime = (currentTime - m_lastFrameTime) / 1000.0f;
    m_lastFrameTime = currentTime;
    
    // Cap delta time to prevent large jumps
    if (m_deltaTime > 0.05f) {
        m_deltaTime = 0.05f;
    }
}

void Game::UpdateFPS() {
    m_frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    
    if (currentTime - m_fpsTimer >= 1000) {
        m_fps = m_frameCount * 1000.0f / (currentTime - m_fpsTimer);
        m_frameCount = 0;
        m_fpsTimer = currentTime;
        
        // Update window title with FPS
        std::string title = "Desktop Survivor Dash - FPS: " + std::to_string((int)m_fps);
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

void Game::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // ImGui event handling
        ImGui_ImplSDL2_ProcessEvent(&event);
        
        // Handle window events
        if (event.type == SDL_QUIT) {
            m_running = false;
        } else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_screenWidth = event.window.data1;
                m_screenHeight = event.window.data2;
                glViewport(0, 0, m_screenWidth, m_screenHeight);
            }
        }
        
        // Pass events to input system
        if (m_input) {
            m_input->HandleEvent(event);
        }
        
        // Pass events to current state
        if (!m_states.empty()) {
            m_states.top()->HandleEvent(event);
        }
    }
}

void Game::Update(float deltaTime) {
    // Update input system
    if (m_input) {
        m_input->Update();
    }
    
    // Update current state
    if (!m_states.empty()) {
        m_states.top()->Update(deltaTime);
    }
}

void Game::Render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render current state
    if (!m_states.empty()) {
        m_states.top()->Render(m_renderer.get());
    }
    
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // Render current state's UI
    if (!m_states.empty()) {
        m_states.top()->RenderUI();
    }
    
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Swap buffers
    SDL_GL_SwapWindow(m_window);
} 