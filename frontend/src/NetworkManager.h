#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

// Forward declarations
struct LeaderboardEntry;
struct Skill;
struct UserCurrency;

// Response structures
struct ApiResponse {
    bool success;
    std::string error;
    std::string data;
};

struct HttpResponse {
    bool success;
    int statusCode;
    std::string data;
    std::string error;
};

// Callback types
using HttpCallback = std::function<void(const HttpResponse& response)>;
using LeaderboardCallback = std::function<void(bool success, const std::vector<LeaderboardEntry>& entries, const std::string& error)>;
using SkillsCallback = std::function<void(bool success, const std::vector<Skill>& skills, const UserCurrency& currency, int userLevel, const std::string& error)>;
using UpgradeCallback = std::function<void(bool success, const std::string& skillId, int newLevel, int remainingSkillPoints, const std::string& error)>;

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Configuration
    void SetBaseUrl(const std::string& baseUrl);
    void SetAuthToken(const std::string& token);
    
    // Leaderboard API calls
    void GetScoreLeaderboard(const std::string& timeframe, int limit, LeaderboardCallback callback);
    void GetSurvivalLeaderboard(int limit, LeaderboardCallback callback);
    void GetKillsLeaderboard(int limit, LeaderboardCallback callback);
    void GetRecentLeaderboard(int limit, LeaderboardCallback callback);
    
    // Skills API calls
    void GetUserSkills(SkillsCallback callback);
    void GetUserStats(HttpCallback callback);
    void UpgradeSkill(const std::string& skillId, UpgradeCallback callback);
    
    // Update method for processing async requests
    void Update();
    
    // Check if network operations are in progress
    bool IsLoading() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    // HTTP request helpers
    ApiResponse MakeRequest(const std::string& endpoint, const std::string& method = "GET", const std::string& body = "");
    void MakeAsyncRequest(const std::string& endpoint, const std::string& method, const std::string& body, std::function<void(const ApiResponse&)> callback);
    
    // JSON parsing helpers
    std::vector<LeaderboardEntry> ParseLeaderboardResponse(const std::string& jsonData);
    std::vector<Skill> ParseSkillsResponse(const std::string& jsonData, UserCurrency& currency, int& userLevel);
}; 