#include "PlayState.h"
#include "HomeState.h"
#include "Game.h"
#include "Renderer.h"
#include "AuthNetworkManager.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <imgui.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <nlohmann/json.hpp>

PlayState::PlayState(Game* game) 
    : GameState(game)
    , m_playerX(640.0f)  // Center of 1280 width
    , m_playerY(360.0f)  // Center of 720 height
    , m_paused(false)
    , m_gameTime(0.0f)
    , m_score(0)
    , m_lives(3)
    , m_leaderboardPoints(0)
    , m_skillPoints(0)
    , m_leaderboardTimer(0.0f)
    , m_skillPointTimer(0.0f)
    , m_saveTimer(0.0f)
    , m_savedGameTime(0.0f)
    , m_savedScore(0)
    , m_savedLeaderboardPoints(0)
    , m_savedSkillPoints(0)
    , m_showPauseMenu(false)
    , m_showGameOver(false)
    , m_canContinue(false)
    , m_sessionId("")
    , m_sessionStarted(false)
    , m_authNetworkManager(std::make_unique<AuthNetworkManager>())
{
}

PlayState::~PlayState() = default;

void PlayState::SetAuthToken(const std::string& token) {
    if (m_authNetworkManager) {
        m_authNetworkManager->SetAuthToken(token);
        std::cout << "Auth token set in PlayState" << std::endl;
    }
}

void PlayState::OnEnter() {
    std::cout << "Starting Desktop Survivor Dash gameplay!" << std::endl;
    std::cout << "Use mouse to move your cursor and survive!" << std::endl;
    std::cout << "Press ESC to pause, Q to quit to menu" << std::endl;
    
    // Start a new game session
    StartGameSession();
}

void PlayState::StartGameSession() {
    if (!m_authNetworkManager) {
        std::cout << "Warning: No network manager available. Playing in offline mode." << std::endl;
        return;
    }

    std::cout << "Starting new game session..." << std::endl;
    
    m_authNetworkManager->StartGameSession([this](const HttpResponse& response) {
        if (response.success) {
            try {
                // Parse session ID from response
                nlohmann::json responseData = nlohmann::json::parse(response.data);
                
                if (responseData.contains("sessionId") && !responseData["sessionId"].is_null()) {
                    m_sessionId = responseData["sessionId"].get<std::string>();
                    m_sessionStarted = true;
                    std::cout << "Game session started successfully! Session ID: " << m_sessionId << std::endl;
                } else {
                    std::cout << "Warning: Server did not return session ID. Playing in offline mode." << std::endl;
                    m_sessionStarted = false;
                }
            } catch (const std::exception& e) {
                std::cout << "Error parsing session response: " << e.what() << std::endl;
                std::cout << "Playing in offline mode..." << std::endl;
                m_sessionStarted = false;
            }
        } else {
            std::cout << "Failed to start game session: " << response.error << std::endl;
            // Try to parse error details from response
            try {
                if (!response.data.empty()) {
                    nlohmann::json errorData = nlohmann::json::parse(response.data);
                    if (errorData.contains("error") && !errorData["error"].is_null()) {
                        std::cout << "Error details: " << errorData["error"].get<std::string>() << std::endl;
                    }
                }
            } catch (...) {
                // Ignore parsing errors for error response
            }
            std::cout << "Continuing in offline mode..." << std::endl;
            m_sessionStarted = false;
        }
    });
}

void PlayState::OnExit() {
    std::cout << "Exiting gameplay. Final stats:" << std::endl;
    std::cout << "  Score: " << m_score << std::endl;
    std::cout << "  Leaderboard Points: " << m_leaderboardPoints << std::endl;
    std::cout << "  Skill Points: " << m_skillPoints << std::endl;
    std::cout << "  Survival Time: " << m_gameTime << " seconds" << std::endl;
    
    // End the session if we haven't already
    if (m_sessionStarted && !m_sessionId.empty()) {
        EndGameSession();
    }
}

void PlayState::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                if (!m_showGameOver) {
                    m_paused = !m_paused;
                    m_showPauseMenu = m_paused;
                }
                break;
            case SDLK_q:
                // Save progress and end session before returning to main menu
                std::cout << "Saving progress before returning to main menu..." << std::endl;
                SaveProgressToServer();
                
                if (m_sessionStarted && !m_sessionId.empty()) {
                    EndGameSession();
                }
                
                // Return to main menu
                m_game->ChangeState(std::make_unique<HomeState>(m_game));
                break;
        }
    } else if (event.type == SDL_MOUSEMOTION && !m_showGameOver) {
        // Update player position to follow mouse
        m_playerX = static_cast<float>(event.motion.x);
        m_playerY = static_cast<float>(event.motion.y);
    }
}

void PlayState::Update(float deltaTime) {
    if (m_paused || m_showGameOver) {
        // Still update network manager even when paused
        if (m_authNetworkManager) {
            m_authNetworkManager->Update();
        }
        return;
    }
    
    // Update network manager
    if (m_authNetworkManager) {
        m_authNetworkManager->Update();
    }
    
    // Update game time
    m_gameTime += deltaTime;
    
    // Update point system (awards points while playing)
    UpdatePointSystem(deltaTime);
    
    // Spawn enemies periodically
    if (static_cast<int>(m_gameTime * 2) > m_enemies.size()) {
        SpawnEnemies();
    }
    
    // Spawn power-ups occasionally
    if (static_cast<int>(m_gameTime / 5) > m_powerUps.size()) {
        SpawnPowerUps();
    }
    
    // Update game entities
    UpdateEnemies(deltaTime);
    UpdatePowerUps(deltaTime);
    
    // Check collisions
    CheckCollisions();
    
    // Update score based on survival time and performance (faster scoring)
    m_score = static_cast<int>(m_gameTime * 25) + (m_enemies.size() * 10) + (m_leaderboardPoints * 2);
    
    // Game over condition
    if (m_lives <= 0) {
        m_showGameOver = true;
        m_canContinue = (m_savedGameTime > 0); // Can continue if we have a saved state
        
        // End the game session with final results
        EndGameSession();
        
        std::cout << "Game Over! Final stats:" << std::endl;
        std::cout << "  Score: " << m_score << std::endl;
        std::cout << "  Leaderboard Points: " << m_leaderboardPoints << std::endl;
        std::cout << "  Skill Points: " << m_skillPoints << std::endl;
        std::cout << "  Survival Time: " << m_gameTime << " seconds" << std::endl;
        if (m_canContinue) {
            std::cout << "Continue option available from " << m_savedGameTime << " seconds" << std::endl;
        }
    }
}

void PlayState::SaveGameState() {
    m_savedGameTime = m_gameTime;
    m_savedScore = m_score;
    m_savedLeaderboardPoints = m_leaderboardPoints;
    m_savedSkillPoints = m_skillPoints;
    m_savedEnemies = m_enemies;
    m_savedPowerUps = m_powerUps;
    std::cout << "Game state saved at " << m_gameTime << " seconds" << std::endl;
}

void PlayState::RestoreGameState() {
    m_gameTime = m_savedGameTime;
    m_score = m_savedScore;
    m_leaderboardPoints = m_savedLeaderboardPoints;
    m_skillPoints = m_savedSkillPoints;
    m_enemies = m_savedEnemies;
    m_powerUps = m_savedPowerUps;
    m_lives = 3; // Restore full lives
    m_showGameOver = false;
    m_paused = false;
    std::cout << "Game state restored to " << m_gameTime << " seconds" << std::endl;
}

void PlayState::RestartGame() {
    // End current session first
    if (m_sessionStarted && !m_sessionId.empty()) {
        EndGameSession();
    }

    // Reset everything to initial state
    m_gameTime = 0.0f;
    m_score = 0;
    m_lives = 3;
    m_leaderboardPoints = 0;
    m_skillPoints = 0;
    m_leaderboardTimer = 0.0f;
    m_skillPointTimer = 0.0f;
    m_saveTimer = 0.0f;
    m_enemies.clear();
    m_powerUps.clear();
    m_savedGameTime = 0.0f;
    m_savedScore = 0;
    m_savedLeaderboardPoints = 0;
    m_savedSkillPoints = 0;
    m_savedEnemies.clear();
    m_savedPowerUps.clear();
    m_showGameOver = false;
    m_paused = false;
    m_canContinue = false;
    
    std::cout << "Game restarted" << std::endl;

    // Start a new session
    StartGameSession();
}

void PlayState::SpawnEnemies() {
    Enemy enemy;
    enemy.active = true;
    enemy.type = rand() % 4;
    enemy.size = 20.0f + (enemy.type * 5.0f);
    
    // Spawn from edges of screen
    int side = rand() % 4;
    switch (side) {
        case 0: // Top
            enemy.x = rand() % 1280;
            enemy.y = -enemy.size;
            enemy.vx = (rand() % 100 - 50) / 10.0f;
            enemy.vy = 50.0f + rand() % 50;
            break;
        case 1: // Right
            enemy.x = 1280 + enemy.size;
            enemy.y = rand() % 720;
            enemy.vx = -(50.0f + rand() % 50);
            enemy.vy = (rand() % 100 - 50) / 10.0f;
            break;
        case 2: // Bottom
            enemy.x = rand() % 1280;
            enemy.y = 720 + enemy.size;
            enemy.vx = (rand() % 100 - 50) / 10.0f;
            enemy.vy = -(50.0f + rand() % 50);
            break;
        case 3: // Left
            enemy.x = -enemy.size;
            enemy.y = rand() % 720;
            enemy.vx = 50.0f + rand() % 50;
            enemy.vy = (rand() % 100 - 50) / 10.0f;
            break;
    }
    
    m_enemies.push_back(enemy);
}

void PlayState::SpawnPowerUps() {
    PowerUp powerUp;
    powerUp.active = true;
    powerUp.type = rand() % 3;
    powerUp.x = 100 + rand() % 1080;
    powerUp.y = 100 + rand() % 520;
    powerUp.pulseTime = 0.0f;
    
    m_powerUps.push_back(powerUp);
}

void PlayState::UpdateEnemies(float deltaTime) {
    for (auto& enemy : m_enemies) {
        if (!enemy.active) continue;
        
        // Move towards player (simple AI)
        float dx = m_playerX - enemy.x;
        float dy = m_playerY - enemy.y;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance > 0) {
            enemy.vx += (dx / distance) * 20.0f * deltaTime;
            enemy.vy += (dy / distance) * 20.0f * deltaTime;
        }
        
        // Apply velocity
        enemy.x += enemy.vx * deltaTime;
        enemy.y += enemy.vy * deltaTime;
        
        // Remove enemies that are too far off screen
        if (enemy.x < -100 || enemy.x > 1380 || enemy.y < -100 || enemy.y > 820) {
            enemy.active = false;
        }
    }
    
    // Remove inactive enemies
    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
        [](const Enemy& e) { return !e.active; }), m_enemies.end());
}

void PlayState::UpdatePowerUps(float deltaTime) {
    for (auto& powerUp : m_powerUps) {
        if (!powerUp.active) continue;
        powerUp.pulseTime += deltaTime;
    }
}

void PlayState::CheckCollisions() {
    // Check enemy collisions
    for (auto& enemy : m_enemies) {
        if (!enemy.active) continue;
        
        if (CircleCollision(m_playerX, m_playerY, 8.0f, enemy.x, enemy.y, enemy.size / 2)) {
            enemy.active = false;
            m_lives--;
            std::cout << "Hit by enemy! Lives remaining: " << m_lives << std::endl;
        }
    }
    
    // Check power-up collisions
    for (auto& powerUp : m_powerUps) {
        if (!powerUp.active) continue;
        
        if (CircleCollision(m_playerX, m_playerY, 8.0f, powerUp.x, powerUp.y, 20.0f)) {
            powerUp.active = false;
            m_score += 50;
            std::cout << "Power-up collected! Score: " << m_score << std::endl;
        }
    }
    
    // Remove inactive power-ups
    m_powerUps.erase(std::remove_if(m_powerUps.begin(), m_powerUps.end(),
        [](const PowerUp& p) { return !p.active; }), m_powerUps.end());
}

bool PlayState::CircleCollision(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < (r1 + r2);
}

void PlayState::Render(Renderer* renderer) {
    // Clear with a desktop-like background (light gray)
    glClearColor(0.9f, 0.9f, 0.95f, 1.0f);
    
    // Draw desktop grid pattern for visual context
    for (int x = 0; x < 1280; x += 64) {
        renderer->DrawRect(x, 0, 1, 720, 0.8f, 0.8f, 0.85f, 0.3f);
    }
    for (int y = 0; y < 720; y += 64) {
        renderer->DrawRect(0, y, 1280, 1, 0.8f, 0.8f, 0.85f, 0.3f);
    }
    
    // Draw actual enemies from the game vector
    for (const auto& enemy : m_enemies) {
        if (!enemy.active) continue;
        
        // Different enemy types based on type
        switch (enemy.type) {
            case 0: // Red error dialog boxes
                renderer->DrawRect(enemy.x - enemy.size/2, enemy.y - enemy.size/2, enemy.size, enemy.size * 0.75f, 0.8f, 0.2f, 0.2f, 0.9f);
                renderer->DrawRect(enemy.x - enemy.size/2 + 2, enemy.y - enemy.size/2 + 2, enemy.size - 4, enemy.size * 0.75f - 4, 1.0f, 0.4f, 0.4f, 0.7f);
                break;
            case 1: // Blue loading circles
                renderer->DrawCircle(enemy.x, enemy.y, enemy.size/2, 0.2f, 0.4f, 0.8f, 0.8f);
                renderer->DrawCircle(enemy.x, enemy.y, enemy.size/3, 0.4f, 0.6f, 1.0f, 0.6f);
                break;
            case 2: // Yellow warning triangles
                renderer->DrawRect(enemy.x - enemy.size/3, enemy.y - enemy.size/2, enemy.size * 0.66f, enemy.size, 0.9f, 0.8f, 0.2f, 0.8f);
                renderer->DrawRect(enemy.x - enemy.size/4, enemy.y - enemy.size/3, enemy.size * 0.5f, enemy.size * 0.66f, 1.0f, 0.9f, 0.4f, 0.6f);
                break;
            case 3: // Green file icons
                renderer->DrawRect(enemy.x - enemy.size/3, enemy.y - enemy.size/2, enemy.size * 0.66f, enemy.size, 0.2f, 0.7f, 0.3f, 0.8f);
                renderer->DrawRect(enemy.x - enemy.size/3 + 3, enemy.y - enemy.size/2 + 3, enemy.size * 0.66f - 6, enemy.size - 6, 0.4f, 0.9f, 0.5f, 0.6f);
                break;
        }
    }
    
    // Draw actual power-ups from the game vector
    for (const auto& powerUp : m_powerUps) {
        if (!powerUp.active) continue;
        
        float pulse = sin(powerUp.pulseTime * 4.0f) * 0.3f + 0.7f;
        
        // Glowing effect with multiple circles
        renderer->DrawCircle(powerUp.x, powerUp.y, 25.0f * pulse, 0.9f, 0.7f, 0.2f, 0.2f);
        renderer->DrawCircle(powerUp.x, powerUp.y, 20.0f * pulse, 1.0f, 0.9f, 0.4f, 0.4f);
        renderer->DrawCircle(powerUp.x, powerUp.y, 15.0f * pulse, 1.0f, 1.0f, 0.8f, 0.6f);
    }
    
    // Draw player cursor as a white arrow-like shape
    renderer->DrawCircle(m_playerX, m_playerY, 8.0f, 0.0f, 0.0f, 0.0f, 1.0f); // Black outline
    renderer->DrawCircle(m_playerX, m_playerY, 6.0f, 1.0f, 1.0f, 1.0f, 1.0f); // White fill
    
    // Draw cursor "trail" for better visibility
    renderer->DrawCircle(m_playerX - 2, m_playerY - 2, 3.0f, 0.8f, 0.8f, 0.8f, 0.5f);
    
    // Draw desktop taskbar at bottom
    renderer->DrawRect(0, 680, 1280, 40, 0.3f, 0.3f, 0.4f, 0.9f);
    renderer->DrawRect(0, 680, 1280, 2, 0.5f, 0.5f, 0.6f, 1.0f);
}

void PlayState::RenderUI() {
    // Game HUD
    if (!m_showPauseMenu && !m_showGameOver) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 140), ImGuiCond_Always);
        
        if (ImGui::Begin("Game HUD", nullptr, 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
            
            // Points display (highlighted)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("🏆 Leaderboard: %d pts", m_leaderboardPoints);
            ImGui::PopStyleColor();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
            ImGui::Text("⚡ Skill Points: %d", m_skillPoints);
            ImGui::PopStyleColor();
            
            ImGui::Separator();
            
            // Game stats
            ImGui::Text("Score: %d", m_score);
            ImGui::Text("Lives: %d", m_lives);
            ImGui::Text("Time: %.1fs", m_gameTime);
            
            ImGui::Separator();
            ImGui::Text("ESC: Pause");
            ImGui::Text("Q: Quit to Menu");
        }
        ImGui::End();
    }
    
    // Pause menu
    if (m_showPauseMenu) {
        ImGui::SetNextWindowPos(ImVec2(400, 200), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_Always);
        
        if (ImGui::Begin("Game Paused", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Game Paused");
            ImGui::Separator();
            
            if (ImGui::Button("Resume", ImVec2(200, 40))) {
                m_paused = false;
                m_showPauseMenu = false;
            }
            
            if (ImGui::Button("Restart Game", ImVec2(200, 40))) {
                RestartGame();
            }
            
            if (ImGui::Button("Main Menu", ImVec2(200, 40))) {
                m_game->ChangeState(std::make_unique<HomeState>(m_game));
            }
        }
        ImGui::End();
    }
    
    // Game over screen
    if (m_showGameOver) {
        ImGui::SetNextWindowPos(ImVec2(350, 200), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Always);
        
        if (ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Game Over!");
            ImGui::Separator();
            ImGui::Text("Final Score: %d", m_score);
            ImGui::Text("Survival Time: %.1f seconds", m_gameTime);
            
            if (m_canContinue) {
                ImGui::Text("Last Save: %.1f seconds", m_savedGameTime);
            }
            
            ImGui::Separator();
            
            // Continue button (only if save state exists)
            if (m_canContinue) {
                if (ImGui::Button("Continue from Save", ImVec2(300, 50))) {
                    RestoreGameState();
                }
                ImGui::Text("Resume from your last checkpoint");
                ImGui::Separator();
            }
            
            // Restart button
            if (ImGui::Button("Restart Game", ImVec2(300, 50))) {
                RestartGame();
            }
            ImGui::Text("Start over from the beginning");
            ImGui::Separator();
            
            // Main menu button
            if (ImGui::Button("Main Menu", ImVec2(300, 50))) {
                m_game->ChangeState(std::make_unique<HomeState>(m_game));
            }
        }
        ImGui::End();
    }
}

void PlayState::UpdatePointSystem(float deltaTime) {
    // Update timers
    m_leaderboardTimer += deltaTime;
    m_skillPointTimer += deltaTime;
    m_saveTimer += deltaTime;
    
    // Award leaderboard points every 0.5 seconds (2 points per second)
    if (m_leaderboardTimer >= 0.5f) {
        m_leaderboardPoints += 1;
        m_leaderboardTimer = 0.0f;
        std::cout << "Leaderboard points: " << m_leaderboardPoints << " (+1)" << std::endl;
    }
    
    // Award skill points every 1 second (1 point per second)
    if (m_skillPointTimer >= 1.0f) {
        m_skillPoints += 1;
        m_skillPointTimer = 0.0f;
        std::cout << "Skill points: " << m_skillPoints << " (+1)" << std::endl;
    }
    
    // Save progress to server every 5 seconds
    if (m_saveTimer >= 5.0f) {
        SaveProgressToServer();
        SaveGameState(); // Also save local game state
        m_saveTimer = 0.0f;
    }
}

void PlayState::SaveProgressToServer() {
    if (!m_authNetworkManager || !m_sessionStarted || m_sessionId.empty()) {
        std::cout << "Cannot save progress - no active session" << std::endl;
        return;
    }
    
    std::cout << "Saving progress to server..." << std::endl;
    std::cout << "  Current Score: " << m_score << std::endl;
    std::cout << "  Leaderboard Points: " << m_leaderboardPoints << std::endl;
    std::cout << "  Skill Points: " << m_skillPoints << std::endl;
    std::cout << "  Survival Time: " << m_gameTime << " seconds" << std::endl;
    std::cout << "  Lives Remaining: " << m_lives << std::endl;
    
    // Save progress to server
    m_authNetworkManager->SaveGameProgress(
        m_sessionId,
        m_score,
        m_leaderboardPoints,
        m_skillPoints,
        m_gameTime,
        m_lives,
        [this](const HttpResponse& response) {
            if (response.success) {
                std::cout << "Progress saved successfully!" << std::endl;
            } else {
                std::cout << "Failed to save progress: " << response.error << std::endl;
            }
        }
    );
}

void PlayState::EndGameSession() {
    if (!m_authNetworkManager || !m_sessionStarted || m_sessionId.empty()) {
        std::cout << "No active session to end" << std::endl;
        return;
    }

    std::cout << "Ending game session..." << std::endl;
    std::cout << "  Final Score: " << m_score << std::endl;
    std::cout << "  Final Leaderboard Points: " << m_leaderboardPoints << std::endl;
    std::cout << "  Final Skill Points: " << m_skillPoints << std::endl;
    std::cout << "  Final Survival Time: " << m_gameTime << " seconds" << std::endl;

    // Calculate kills and damage for session stats
    int totalKills = 0; // TODO: Track actual kills
    int damageDealt = 0; // TODO: Track damage dealt
    int damageTaken = (3 - m_lives) * 100; // Estimate damage taken based on lives lost
    int waveReached = static_cast<int>(m_gameTime / 30) + 1; // Estimate wave based on time

    m_authNetworkManager->EndGameSession(
        m_sessionId,
        m_score,
        m_leaderboardPoints,
        m_skillPoints,
        m_gameTime,
        totalKills,
        damageDealt,
        damageTaken,
        waveReached,
        [this](const HttpResponse& response) {
            if (response.success) {
                std::cout << "Game session ended successfully!" << std::endl;
                // TODO: Parse and display rewards/bonuses from response
            } else {
                std::cout << "Failed to end game session: " << response.error << std::endl;
            }
            
            // Mark session as ended regardless of success
            m_sessionStarted = false;
            m_sessionId.clear();
        }
    );
} 