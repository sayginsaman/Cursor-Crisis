#pragma once

#include "GameState.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

// Forward declarations
class NetworkManager;

// Data structures for leaderboards
struct LeaderboardEntry {
    int rank;
    std::string username;
    int level;
    std::string avatar;
    int score;
    int survivalTime;
    int kills;
    std::string achievedAt;
};

// Data structures for skills
struct SkillEffect {
    std::string type;
    float currentValue;
    float nextValue;
    bool isPercentage;
};

struct Skill {
    std::string skillId;
    std::string name;
    std::string description;
    std::string category;
    std::string icon;
    int maxLevel;
    int currentLevel;
    bool isUnlocked;
    bool prerequisitesMet;
    bool canUpgrade;
    int nextLevelCost;
    std::vector<SkillEffect> effects;
    int unlockLevel;
};

struct UserCurrency {
    int skillPoints;
    int coins;
};

class HomeState : public GameState {
public:
    HomeState(Game* game);
    ~HomeState() override;

    void OnEnter() override;
    void OnExit() override;
    
    void HandleEvent(const SDL_Event& event) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;
    void RenderUI() override;
    
    // Authentication
    void SetAuthToken(const std::string& token);

private:
    // UI State Management
    enum class UIMode {
        MAIN_MENU,
        LEADERBOARDS,
        SKILLS,
        SETTINGS,
        PROFILE
    };
    
    UIMode m_currentMode;
    
    // Menu states
    bool m_showMainMenu;
    bool m_showLeaderboards;
    bool m_showSkills;
    bool m_showSettings;
    bool m_showProfile;
    
    // Leaderboard data
    std::vector<LeaderboardEntry> m_scoreLeaderboard;
    std::vector<LeaderboardEntry> m_survivalLeaderboard;
    std::vector<LeaderboardEntry> m_killsLeaderboard;
    std::vector<LeaderboardEntry> m_recentLeaderboard;
    std::string m_selectedLeaderboardTab;
    bool m_leaderboardLoading;
    std::string m_leaderboardError;
    
    // Skills data
    std::vector<Skill> m_skills;
    std::map<std::string, std::vector<Skill*>> m_skillsByCategory;
    UserCurrency m_userCurrency;
    std::string m_selectedSkillCategory;
    bool m_skillsLoading;
    std::string m_skillsError;
    int m_userLevel;
    
    // Settings
    float m_volume;
    bool m_fullscreen;
    std::string m_selectedGraphicsQuality;
    
    // Network management
    std::unique_ptr<NetworkManager> m_networkManager;
    
    // Authentication
    std::string m_authToken;
    
    // UI rendering methods
    void RenderMainMenu();
    void RenderLeaderboards();
    void RenderSkills();
    void RenderSettings();
    void RenderProfile();
    
    // Leaderboard methods
    void LoadLeaderboards();
    void RenderLeaderboardTab(const std::string& tabName, const std::vector<LeaderboardEntry>& entries);
    
    // Skills methods
    void LoadSkills();
    void LoadUserProgress();
    void RenderSkillsCategory(const std::string& category);
    void RenderSkillCard(const Skill& skill);
    void UpgradeSkill(const std::string& skillId);
    
    // Utility methods
    void SetUIMode(UIMode mode);
    std::string FormatTime(int milliseconds);
    std::string FormatTimeAgo(const std::string& timestamp);
    void ShowNotification(const std::string& message, float duration = 3.0f);
    
    // Navigation
    void NavigateToPlay();
    void NavigateToSettings();
    void NavigateToProfile();
    void ExitGame();
}; 