#pragma once

#include "GameState.h"
#include <vector>
#include <memory>
#include <string>

// Forward declarations
class AuthNetworkManager;

struct Enemy {
    float x, y;
    float vx, vy;
    int type;
    bool active;
    float size;
};

struct PowerUp {
    float x, y;
    bool active;
    int type;
    float pulseTime;
};

class PlayState : public GameState {
public:
    PlayState(Game* game);
    ~PlayState() override;

    void OnEnter() override;
    void OnExit() override;
    
    void HandleEvent(const SDL_Event& event) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;
    void RenderUI() override;
    
    // Authentication
    void SetAuthToken(const std::string& token);

private:
    // Player cursor position
    float m_playerX;
    float m_playerY;
    
    // Game state
    bool m_paused;
    float m_gameTime;
    int m_score;
    int m_lives;
    
    // Point system
    int m_leaderboardPoints;    // Points for leaderboard (2 per second)
    int m_skillPoints;          // Points for skills (1 per 2 seconds)
    float m_leaderboardTimer;   // Timer for leaderboard points (1 second intervals)
    float m_skillPointTimer;    // Timer for skill points (2 second intervals)
    float m_saveTimer;          // Timer for saving progress (every 5 seconds)
    
    // Session management
    std::string m_sessionId;    // Current game session ID
    bool m_sessionStarted;      // Whether a session is active
    
    // Game state backup for continue feature
    float m_savedGameTime;
    int m_savedScore;
    int m_savedLeaderboardPoints;
    int m_savedSkillPoints;
    std::vector<Enemy> m_savedEnemies;
    std::vector<PowerUp> m_savedPowerUps;
    
    // Game entities
    std::vector<Enemy> m_enemies;
    std::vector<PowerUp> m_powerUps;
    
    // UI state
    bool m_showPauseMenu;
    bool m_showGameOver;
    bool m_canContinue;
    
    // Network
    std::unique_ptr<AuthNetworkManager> m_authNetworkManager;
    
    // Game mechanics
    void SpawnEnemies();
    void SpawnPowerUps();
    void UpdateEnemies(float deltaTime);
    void UpdatePowerUps(float deltaTime);
    void CheckCollisions();
    bool CircleCollision(float x1, float y1, float r1, float x2, float y2, float r2);
    
    // Point system
    void UpdatePointSystem(float deltaTime);
    void SaveProgressToServer();
    
    // Session management
    void StartGameSession();
    void EndGameSession();
    
    // Save/restore game state
    void SaveGameState();
    void RestoreGameState();
    void RestartGame();
}; 