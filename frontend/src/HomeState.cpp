#include "HomeState.h"
#include "PlayState.h"
#include "Game.h"
#include "Renderer.h"
#include "NetworkManager.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

HomeState::HomeState(Game* game) 
    : GameState(game)
    , m_currentMode(UIMode::MAIN_MENU)
    , m_showMainMenu(true)
    , m_showLeaderboards(false)
    , m_showSkills(false)
    , m_showSettings(false)
    , m_showProfile(false)
    , m_selectedLeaderboardTab("scores")
    , m_leaderboardLoading(false)
    , m_selectedSkillCategory("combat")
    , m_skillsLoading(false)
    , m_userLevel(1)
    , m_volume(0.8f)
    , m_fullscreen(false)
    , m_selectedGraphicsQuality("medium")
    , m_networkManager(std::make_unique<NetworkManager>())
{
    // Initialize with starting values - will be updated from real user data
    m_userCurrency.skillPoints = 0;
    m_userCurrency.coins = 50;
    
    // Initialize empty data - will be populated from API calls
    m_scoreLeaderboard.clear();
    m_skills.clear();
}

HomeState::~HomeState() = default;

void HomeState::OnEnter() {
    std::cout << "Entering Home State - Loading user progress..." << std::endl;
    
    // Clear sample data but keep user's actual progress
    m_scoreLeaderboard.clear();
    m_survivalLeaderboard.clear();
    m_killsLeaderboard.clear();
    m_recentLeaderboard.clear();
    m_skills.clear();
    m_skillsByCategory.clear();
    
    // Load user's current stats from database
    LoadUserProgress();
    
    // Load real data from network
    LoadLeaderboards();
    LoadSkills();
}

void HomeState::OnExit() {
    std::cout << "Exiting Home State" << std::endl;
}

void HomeState::SetAuthToken(const std::string& token) {
    m_authToken = token;
    std::cout << "Auth token set in HomeState" << std::endl;
}

void HomeState::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                if (m_currentMode != UIMode::MAIN_MENU) {
                    SetUIMode(UIMode::MAIN_MENU);
                } else {
                    ExitGame();
                }
                break;
            case SDLK_F1:
                SetUIMode(UIMode::LEADERBOARDS);
                break;
            case SDLK_F2:
                SetUIMode(UIMode::SKILLS);
                break;
        }
    }
}

void HomeState::Update(float deltaTime) {
    m_networkManager->Update();
}

void HomeState::Render(Renderer* renderer) {
    renderer->DrawRect(0, 0, 1280, 720, 0.1f, 0.1f, 0.15f, 1.0f);
    renderer->DrawRect(0, 680, 1280, 40, 0.2f, 0.2f, 0.3f, 0.9f);
}

void HomeState::RenderUI() {
    switch (m_currentMode) {
        case UIMode::MAIN_MENU:
            RenderMainMenu();
            break;
        case UIMode::LEADERBOARDS:
            RenderLeaderboards();
            break;
        case UIMode::SKILLS:
            RenderSkills();
            break;
        case UIMode::SETTINGS:
            RenderSettings();
            break;
        case UIMode::PROFILE:
            RenderProfile();
            break;
    }
}

void HomeState::RenderMainMenu() {
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Always);
    
    if (ImGui::Begin("Desktop Survivor Dash", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Welcome, Cursor Warrior!");
        ImGui::Separator();
        
        ImGui::Text("Level: %d", m_userLevel);
        ImGui::Text("Skill Points: %d", m_userCurrency.skillPoints);
        ImGui::Text("Coins: %d", m_userCurrency.coins);
        ImGui::Separator();
        
        if (ImGui::Button("Play Game", ImVec2(250, 50))) {
            NavigateToPlay();
        }
        
        if (ImGui::Button("Leaderboards", ImVec2(250, 40))) {
            SetUIMode(UIMode::LEADERBOARDS);
        }
        
        if (ImGui::Button("Skills & Upgrades", ImVec2(250, 40))) {
            SetUIMode(UIMode::SKILLS);
        }
        
        if (ImGui::Button("Exit Game", ImVec2(250, 40))) {
            ExitGame();
        }
        
        ImGui::Separator();
        ImGui::Text("F1 - Leaderboards");
        ImGui::Text("F2 - Skills");
    }
    ImGui::End();
    
    // Quick leaderboard preview
    ImGui::SetNextWindowPos(ImVec2(400, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Always);
    
    if (ImGui::Begin("Top Scores", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        if (m_scoreLeaderboard.empty()) {
            ImGui::Text("No scores yet - be the first!");
            ImGui::Text("Play games to appear on leaderboards");
        } else {
            for (size_t i = 0; i < 5 && i < m_scoreLeaderboard.size(); ++i) {
                const auto& entry = m_scoreLeaderboard[i];
                ImGui::Text("#%d %s - %d pts", entry.rank, entry.username.c_str(), entry.score);
            }
        }
        
        if (ImGui::Button("View Full Leaderboards")) {
            SetUIMode(UIMode::LEADERBOARDS);
        }
    }
    ImGui::End();
    
    // Skills preview
    ImGui::SetNextWindowPos(ImVec2(850, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(380, 300), ImGuiCond_Always);
    
    if (ImGui::Begin("Your Skills", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Skill Points: %d", m_userCurrency.skillPoints);
        ImGui::Separator();
        
        if (m_skills.empty()) {
            ImGui::Text("No skills available yet");
            ImGui::Text("Play games to earn skill points");
            ImGui::Text("and unlock new abilities!");
        } else {
            for (const auto& skill : m_skills) {
                if (skill.canUpgrade) {
                    ImGui::Text("%s (Lv.%d)", skill.name.c_str(), skill.currentLevel);
                    ImGui::SameLine();
                    if (ImGui::SmallButton(("Up##" + skill.skillId).c_str())) {
                        UpgradeSkill(skill.skillId);
                    }
                }
            }
        }
        
        if (ImGui::Button("View All Skills")) {
            SetUIMode(UIMode::SKILLS);
        }
    }
    ImGui::End();
}

void HomeState::RenderLeaderboards() {
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(1180, 600), ImGuiCond_Always);
    
    if (ImGui::Begin("Global Leaderboards", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        if (ImGui::Button("< Back")) {
            SetUIMode(UIMode::MAIN_MENU);
        }
        
        ImGui::Separator();
        
        if (ImGui::BeginTable("Leaderboard", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Rank");
            ImGui::TableSetupColumn("Player");
            ImGui::TableSetupColumn("Level");
            ImGui::TableSetupColumn("Score");
            ImGui::TableSetupColumn("Time");
            ImGui::TableHeadersRow();
            
            for (const auto& entry : m_scoreLeaderboard) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                
                if (entry.rank <= 3) {
                    ImVec4 color = (entry.rank == 1) ? ImVec4(1.0f, 0.8f, 0.0f, 1.0f) : 
                                  (entry.rank == 2) ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : 
                                                     ImVec4(0.8f, 0.5f, 0.2f, 1.0f);
                    ImGui::TextColored(color, "#%d", entry.rank);
                } else {
                    ImGui::Text("%d", entry.rank);
                }
                
                ImGui::TableNextColumn();
                ImGui::Text("%s", entry.username.c_str());
                
                ImGui::TableNextColumn();
                ImGui::Text("%d", entry.level);
                
                ImGui::TableNextColumn();
                ImGui::Text("%d", entry.score);
                
                ImGui::TableNextColumn();
                ImGui::Text("%s", FormatTime(entry.survivalTime).c_str());
            }
            
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void HomeState::RenderSkills() {
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(1180, 600), ImGuiCond_Always);
    
    if (ImGui::Begin("Skills & Upgrades", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        if (ImGui::Button("< Back")) {
            SetUIMode(UIMode::MAIN_MENU);
        }
        
        ImGui::SameLine();
        ImGui::Text("Skill Points: %d", m_userCurrency.skillPoints);
        
        ImGui::Separator();
        
        for (const auto& skill : m_skills) {
            ImGui::BeginGroup();
            
            ImGui::Text("%s (Level %d/%d)", skill.name.c_str(), skill.currentLevel, skill.maxLevel);
            ImGui::Text("%s", skill.description.c_str());
            
            if (skill.canUpgrade && m_userCurrency.skillPoints >= skill.nextLevelCost) {
                if (ImGui::Button(("Upgrade (" + std::to_string(skill.nextLevelCost) + " SP)").c_str())) {
                    UpgradeSkill(skill.skillId);
                }
            } else if (skill.currentLevel >= skill.maxLevel) {
                ImGui::Text("MAX LEVEL");
            } else {
                ImGui::Text("Need %d skill points", skill.nextLevelCost);
            }
            
            ImGui::Separator();
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

void HomeState::RenderSettings() {
    // Simple settings implementation
}

void HomeState::RenderProfile() {
    // Simple profile implementation
}

void HomeState::RenderLeaderboardTab(const std::string& tabName, const std::vector<LeaderboardEntry>& entries) {
    // Implementation for different leaderboard tabs
}

void HomeState::SetUIMode(UIMode mode) {
    m_currentMode = mode;
}

void HomeState::LoadLeaderboards() {
    // Clear any existing data first
    m_scoreLeaderboard.clear();
    m_survivalLeaderboard.clear(); 
    m_killsLeaderboard.clear();
    m_recentLeaderboard.clear();
    
    // TODO: Load real leaderboard data from network
    // For now, keep empty - no sample data
    std::cout << "Leaderboards cleared - no sample data loaded" << std::endl;
}

void HomeState::LoadSkills() {
    // Clear any existing data first
    m_skills.clear();
    m_skillsByCategory.clear();
    
    // TODO: Load real skills data from network  
    // For now, keep empty - no sample data
    std::cout << "Skills cleared - no sample data loaded" << std::endl;
}

void HomeState::UpgradeSkill(const std::string& skillId) {
    for (auto& skill : m_skills) {
        if (skill.skillId == skillId && skill.canUpgrade && m_userCurrency.skillPoints >= skill.nextLevelCost) {
            m_userCurrency.skillPoints -= skill.nextLevelCost;
            skill.currentLevel++;
            
            if (skill.currentLevel >= skill.maxLevel) {
                skill.canUpgrade = false;
            } else {
                skill.nextLevelCost = static_cast<int>(skill.nextLevelCost * 1.5f);
            }
            
            std::cout << "Upgraded " << skill.name << " to level " << skill.currentLevel << std::endl;
            break;
        }
    }
}

void HomeState::RenderSkillsCategory(const std::string& category) {
    // Category filtering implementation
}

void HomeState::RenderSkillCard(const Skill& skill) {
    // Skill card rendering implementation
}

void HomeState::NavigateToPlay() {
    auto playState = std::make_unique<PlayState>(m_game);
    playState->SetAuthToken(m_authToken);
    m_game->ChangeState(std::move(playState));
}

void HomeState::NavigateToSettings() {
    SetUIMode(UIMode::SETTINGS);
}

void HomeState::NavigateToProfile() {
    SetUIMode(UIMode::PROFILE);
}

void HomeState::ExitGame() {
    m_game->SetRunning(false);
}

std::string HomeState::FormatTime(int milliseconds) {
    int seconds = milliseconds / 1000;
    int minutes = seconds / 60;
    seconds %= 60;
    return std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
}

std::string HomeState::FormatTimeAgo(const std::string& timestamp) {
    return "2h ago";
}

void HomeState::ShowNotification(const std::string& message, float duration) {
    std::cout << "Notification: " << message << std::endl;
}

void HomeState::LoadUserProgress() {
    if (!m_networkManager) {
        std::cout << "No network manager available - using default values" << std::endl;
        m_userLevel = 1;
        m_userCurrency.skillPoints = 0;
        m_userCurrency.coins = 50;
        return;
    }
    
    std::cout << "Loading user progress from server..." << std::endl;
    
    // Make API call to get current user stats
    m_networkManager->GetUserStats([this](const HttpResponse& response) {
        if (response.success) {
            try {
                // Parse user stats from response
                // For now, simulate loading saved progress
                std::cout << "User progress loaded successfully!" << std::endl;
                // TODO: Parse actual JSON response and update:
                // m_userLevel = parsed.level;
                // m_userCurrency.skillPoints = parsed.skill_points;
                // m_userCurrency.coins = parsed.coins;
                
                std::cout << "Current stats:" << std::endl;
                std::cout << "  Level: " << m_userLevel << std::endl;
                std::cout << "  Skill Points: " << m_userCurrency.skillPoints << std::endl;
                std::cout << "  Coins: " << m_userCurrency.coins << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Failed to parse user stats: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Failed to load user progress: " << response.error << std::endl;
            std::cout << "Using default values" << std::endl;
        }
    });
} 