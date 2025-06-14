#include "MenuState.h"
#include "PlayState.h"
#include "Game.h"
#include <SDL2/SDL.h>
#include <imgui.h>
#include <iostream>

MenuState::MenuState(Game* game) 
    : GameState(game)
    , m_showMainMenu(true)
    , m_showSettings(false)
    , m_showCredits(false)
{
}

void MenuState::OnEnter() {
    std::cout << "Entering Main Menu" << std::endl;
}

void MenuState::OnExit() {
    std::cout << "Exiting Main Menu" << std::endl;
}

void MenuState::HandleEvent(const SDL_Event& event) {
    // Handle menu-specific events
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            m_game->SetRunning(false);
        }
    }
}

void MenuState::Update(float /*deltaTime*/) {
    // Update menu logic
}

void MenuState::Render(Renderer* /*renderer*/) {
    // Render menu background
}

void MenuState::RenderUI() {
    if (m_showMainMenu) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        
        if (ImGui::Begin("Desktop Survivor Dash", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::Text("Welcome to Desktop Survivor Dash!");
            ImGui::Separator();
            
            if (ImGui::Button("Start Game", ImVec2(200, 40))) {
                // Transition to PlayState
                m_game->ChangeState(std::make_unique<PlayState>(m_game));
            }
            
            if (ImGui::Button("Settings", ImVec2(200, 40))) {
                m_showSettings = true;
            }
            
            if (ImGui::Button("Credits", ImVec2(200, 40))) {
                m_showCredits = true;
            }
            
            if (ImGui::Button("Exit", ImVec2(200, 40))) {
                m_game->SetRunning(false);
            }
        }
        ImGui::End();
    }
    
    if (m_showSettings) {
        if (ImGui::Begin("Settings", &m_showSettings)) {
            ImGui::Text("Game Settings");
            ImGui::Separator();
            
            // Add settings controls here
            static float volume = 0.8f;
            ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
            
            static bool fullscreen = false;
            ImGui::Checkbox("Fullscreen", &fullscreen);
            
            if (ImGui::Button("Apply")) {
                // Apply settings
            }
        }
        ImGui::End();
    }
    
    if (m_showCredits) {
        if (ImGui::Begin("Credits", &m_showCredits)) {
            ImGui::Text("Desktop Survivor Dash");
            ImGui::Text("A roguelite cursor survival game");
            ImGui::Separator();
            ImGui::Text("Developed with C++, SDL2, and ImGui");
        }
        ImGui::End();
    }
} 