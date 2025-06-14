#pragma once

#include <string>
#include <functional>
#include <memory>
#include "NetworkManager.h"

// Authentication response structures
struct AuthResponse {
    bool success;
    std::string error;
    std::string token;
    std::string userId;
    std::string username;
    std::string email;
    std::string steamId;
    std::string authMethod;
    int level;
    int experience;
    int skillPoints;
    int coins;
};

// Callback types
using AuthCallback = std::function<void(const AuthResponse& response)>;
using HttpCallback = std::function<void(const HttpResponse& response)>;

class AuthNetworkManager {
public:
    AuthNetworkManager();
    ~AuthNetworkManager();

    // Configuration
    void SetBaseUrl(const std::string& baseUrl);
    void SetAuthToken(const std::string& token);
    
    // Authentication API calls
    void RegisterEmailUser(const std::string& username, const std::string& email, 
                          const std::string& password, AuthCallback callback);
    
    void LoginEmailUser(const std::string& email, const std::string& password, 
                       AuthCallback callback);
    
    void CreateSteamUser(const std::string& steamId, const std::string& username,
                        const std::string& avatar, AuthCallback callback);
    
    void LoginSteamUser(const std::string& steamId, AuthCallback callback);
    
    void CreateLinkedUser(const std::string& username, const std::string& email,
                         const std::string& password, const std::string& steamId,
                         const std::string& avatar, AuthCallback callback);
    
    // Email checking
    void CheckEmailExists(const std::string& email, std::function<void(bool exists, const std::string& error)> callback);
    
    // Game progress API calls
    void StartGameSession(HttpCallback callback);
    
    void SaveGameProgress(const std::string& sessionId, int currentScore, 
                         int leaderboardPoints, int skillPoints, float survivalTime, 
                         int livesRemaining, HttpCallback callback);
    
    void SaveProgress(int leaderboardPoints, int skillPoints, int currentScore, 
                     float survivalTime, HttpCallback callback);
    
    void GetProgress(HttpCallback callback);
    
    void EndGameSession(const std::string& sessionId, int finalScore, 
                       int finalLeaderboardPoints, int finalSkillPoints,
                       float survivalTime, int kills, int damageDealt, 
                       int damageTaken, int waveReached, HttpCallback callback);
    
    // Update method for processing async requests
    void Update();
    
    // Check if network operations are in progress
    bool IsLoading() const;

    // Make HTTP request (public for development authentication)
    void MakeHttpRequest(const std::string& endpoint, const std::string& method,
                        const std::string& body, HttpCallback callback);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    // HTTP request helpers
    void MakeAuthRequest(const std::string& endpoint, const std::string& jsonBody, 
                        AuthCallback callback, const std::string& method = "POST");
    
    // JSON parsing helpers
    AuthResponse ParseAuthResponse(const std::string& jsonData);
    std::string CreateAuthJson(const std::string& authMethod, const std::string& username = "",
                              const std::string& email = "", const std::string& password = "",
                              const std::string& steamId = "", const std::string& steamData = "");
}; 