#include "AuthNetworkManager.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

// Structure for write callback
struct WriteCallbackData {
    std::string data;
};

// CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, WriteCallbackData* userp) {
    size_t realsize = size * nmemb;
    userp->data.append((char*)contents, realsize);
    return realsize;
}

// Request structure for async processing
struct AuthRequest {
    std::string url;
    std::string method;
    std::string body;
    std::string headers;
    AuthCallback authCallback;
    HttpCallback httpCallback;
    bool isAuthRequest;
};

// Implementation details
struct AuthNetworkManager::Impl {
    std::string baseUrl;
    std::string authToken;
    std::queue<AuthRequest> requestQueue;
    std::mutex queueMutex;
    bool isLoading;
    
    Impl() : baseUrl("http://localhost:3001"), isLoading(false) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    ~Impl() {
        curl_global_cleanup();
    }
};

AuthNetworkManager::AuthNetworkManager() : m_impl(std::make_unique<Impl>()) {}

AuthNetworkManager::~AuthNetworkManager() = default;

void AuthNetworkManager::SetBaseUrl(const std::string& baseUrl) {
    m_impl->baseUrl = baseUrl;
}

void AuthNetworkManager::SetAuthToken(const std::string& token) {
    m_impl->authToken = token;
}

void AuthNetworkManager::RegisterEmailUser(const std::string& username, const std::string& email, 
                                          const std::string& password, AuthCallback callback) {
    std::string jsonBody = CreateAuthJson("email", username, email, password);
    MakeAuthRequest("/api/auth/choose-method", jsonBody, callback);
}

void AuthNetworkManager::LoginEmailUser(const std::string& email, const std::string& password, 
                                       AuthCallback callback) {
    std::string jsonBody = CreateAuthJson("login", "", email, password);
    MakeAuthRequest("/api/auth/login", jsonBody, callback);
}

void AuthNetworkManager::CreateSteamUser(const std::string& steamId, const std::string& username,
                                        const std::string& avatar, AuthCallback callback) {
    std::string steamData = "{\"username\":\"" + username + "\",\"avatar\":\"" + avatar + "\"}";
    std::string jsonBody = CreateAuthJson("steam", "", "", "", steamId, steamData);
    MakeAuthRequest("/api/auth/choose-method", jsonBody, callback);
}

void AuthNetworkManager::LoginSteamUser(const std::string& steamId, AuthCallback callback) {
    std::string jsonBody = CreateAuthJson("steam", "", "", "", steamId);
    MakeAuthRequest("/api/auth/steam", jsonBody, callback);
}

void AuthNetworkManager::CreateLinkedUser(const std::string& username, const std::string& email,
                                         const std::string& password, const std::string& steamId,
                                         const std::string& avatar, AuthCallback callback) {
    std::string steamData = "{\"username\":\"" + username + "\",\"avatar\":\"" + avatar + "\"}";
    std::string jsonBody = CreateAuthJson("linked", username, email, password, steamId, steamData);
    MakeAuthRequest("/api/auth/choose-method", jsonBody, callback);
}

void AuthNetworkManager::CheckEmailExists(const std::string& email, 
                                         std::function<void(bool exists, const std::string& error)> callback) {
    // Make GET request to check email endpoint
    std::string endpoint = "/api/auth/check-email?email=" + email;
    
    MakeHttpRequest(endpoint, "GET", "", [callback](const HttpResponse& response) {
        if (response.success) {
            try {
                // Parse JSON response
                json root = json::parse(response.data);
                bool exists = root.contains("exists") && root["exists"].get<bool>();
                callback(exists, "");
            } catch (const std::exception& e) {
                callback(false, "Failed to parse response");
            }
        } else {
            callback(false, response.error);
        }
    });
}

void AuthNetworkManager::Update() {
    std::lock_guard<std::mutex> lock(m_impl->queueMutex);
    
    if (!m_impl->requestQueue.empty()) {
        AuthRequest request = m_impl->requestQueue.front();
        m_impl->requestQueue.pop();
        
        // Process request asynchronously
        std::thread([this, request]() {
            m_impl->isLoading = true;
            
            CURL* curl = curl_easy_init();
            if (curl) {
                WriteCallbackData writeData;
                
                // Set URL
                std::string fullUrl = m_impl->baseUrl + request.url;
                curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
                
                // Set method
                if (request.method == "POST") {
                    curl_easy_setopt(curl, CURLOPT_POST, 1L);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                } else if (request.method == "GET") {
                    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
                }
                
                // Set headers
                struct curl_slist* headers = nullptr;
                headers = curl_slist_append(headers, "Content-Type: application/json");
                if (!m_impl->authToken.empty()) {
                    std::string authHeader = "Authorization: Bearer " + m_impl->authToken;
                    headers = curl_slist_append(headers, authHeader.c_str());
                }
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                
                // Set write callback
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
                
                // Perform request
                CURLcode res = curl_easy_perform(curl);
                
                long httpCode = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
                
                // Clean up
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                
                // Process response
                if (res == CURLE_OK) {
                    if (request.isAuthRequest) {
                        AuthResponse authResponse = ParseAuthResponse(writeData.data);
                        authResponse.success = (httpCode >= 200 && httpCode < 300);
                        if (!authResponse.success && authResponse.error.empty()) {
                            authResponse.error = "HTTP " + std::to_string(httpCode);
                        }
                        request.authCallback(authResponse);
                    } else {
                        HttpResponse httpResponse;
                        httpResponse.success = (httpCode >= 200 && httpCode < 300);
                        httpResponse.statusCode = httpCode;
                        httpResponse.data = writeData.data;
                        if (!httpResponse.success) {
                            httpResponse.error = "HTTP " + std::to_string(httpCode);
                        }
                        request.httpCallback(httpResponse);
                    }
                } else {
                    if (request.isAuthRequest) {
                        AuthResponse authResponse;
                        authResponse.success = false;
                        authResponse.error = curl_easy_strerror(res);
                        request.authCallback(authResponse);
                    } else {
                        HttpResponse httpResponse;
                        httpResponse.success = false;
                        httpResponse.error = curl_easy_strerror(res);
                        request.httpCallback(httpResponse);
                    }
                }
            }
            
            m_impl->isLoading = false;
        }).detach();
    }
}

bool AuthNetworkManager::IsLoading() const {
    return m_impl->isLoading;
}

void AuthNetworkManager::MakeAuthRequest(const std::string& endpoint, const std::string& jsonBody, 
                                        AuthCallback callback, const std::string& method) {
    AuthRequest request;
    request.url = endpoint;
    request.method = method;
    request.body = jsonBody;
    request.authCallback = callback;
    request.isAuthRequest = true;
    
    std::lock_guard<std::mutex> lock(m_impl->queueMutex);
    m_impl->requestQueue.push(request);
}

void AuthNetworkManager::MakeHttpRequest(const std::string& endpoint, const std::string& method,
                                        const std::string& body, HttpCallback callback) {
    AuthRequest request;
    request.url = endpoint;
    request.method = method;
    request.body = body;
    request.httpCallback = callback;
    request.isAuthRequest = false;
    
    std::lock_guard<std::mutex> lock(m_impl->queueMutex);
    m_impl->requestQueue.push(request);
}

AuthResponse AuthNetworkManager::ParseAuthResponse(const std::string& jsonData) {
    AuthResponse response;
    response.success = false;
    
    try {
        json root = json::parse(jsonData);
        
        response.success = root.contains("success") && root["success"].get<bool>();
        response.error = root.contains("error") ? root["error"].get<std::string>() : "";
        response.token = root.contains("token") ? root["token"].get<std::string>() : "";
        
        if (root.contains("user")) {
            json user = root["user"];
            response.userId = user.contains("id") ? user["id"].get<std::string>() : "";
            response.username = user.contains("username") ? user["username"].get<std::string>() : "";
            response.email = user.contains("email") ? user["email"].get<std::string>() : "";
            response.steamId = user.contains("steam_id") ? user["steam_id"].get<std::string>() : "";
            response.authMethod = user.contains("auth_method") ? user["auth_method"].get<std::string>() : "";
            response.level = user.contains("level") ? user["level"].get<int>() : 1;
            response.experience = user.contains("experience") ? user["experience"].get<int>() : 0;
            response.skillPoints = user.contains("skill_points") ? user["skill_points"].get<int>() : 0;
            response.coins = user.contains("coins") ? user["coins"].get<int>() : 50;
        }
    } catch (const std::exception& e) {
        response.error = "Failed to parse server response";
    }
    
    return response;
}

void AuthNetworkManager::SaveProgress(int leaderboardPoints, int skillPoints, int currentScore, 
                                     float survivalTime, HttpCallback callback) {
    json requestBody;
    requestBody["leaderboard_points"] = leaderboardPoints;
    requestBody["skill_points"] = skillPoints;
    requestBody["current_score"] = currentScore;
    requestBody["survival_time"] = survivalTime;
    
    MakeHttpRequest("/api/game/save-progress", "POST", requestBody.dump(), callback);
}

void AuthNetworkManager::GetProgress(HttpCallback callback) {
    MakeHttpRequest("/api/game/progress", "GET", "", callback);
}

void AuthNetworkManager::StartGameSession(HttpCallback callback) {
    json requestBody;
    requestBody["gameMode"] = "normal";
    MakeHttpRequest("/api/game/session/start", "POST", requestBody.dump(), callback);
}

void AuthNetworkManager::SaveGameProgress(const std::string& sessionId, int currentScore, 
                                         int leaderboardPoints, int skillPoints, float survivalTime, 
                                         int livesRemaining, HttpCallback callback) {
    json requestBody;
    requestBody["sessionId"] = sessionId;
    requestBody["currentScore"] = currentScore;
    requestBody["leaderboardPoints"] = leaderboardPoints;
    requestBody["skillPoints"] = skillPoints;
    requestBody["survivalTime"] = survivalTime;
    requestBody["livesRemaining"] = livesRemaining;
    
    MakeHttpRequest("/api/game/progress/save", "POST", requestBody.dump(), callback);
}

void AuthNetworkManager::EndGameSession(const std::string& sessionId, int finalScore, 
                                       int finalLeaderboardPoints, int finalSkillPoints,
                                       float survivalTime, int kills, int damageDealt, 
                                       int damageTaken, int waveReached, HttpCallback callback) {
    json requestBody;
    requestBody["sessionId"] = sessionId;
    requestBody["finalScore"] = finalScore;
    requestBody["leaderboardPointsEarned"] = finalLeaderboardPoints;
    requestBody["skillPointsEarned"] = finalSkillPoints;
    requestBody["survivalTime"] = survivalTime;
    requestBody["kills"] = kills;
    requestBody["damageDealt"] = damageDealt;
    requestBody["damageTaken"] = damageTaken;
    requestBody["waveReached"] = waveReached;
    requestBody["endReason"] = "player_death";
    
    MakeHttpRequest("/api/game/session/end", "POST", requestBody.dump(), callback);
}

std::string AuthNetworkManager::CreateAuthJson(const std::string& authMethod, const std::string& username,
                                              const std::string& email, const std::string& password,
                                              const std::string& steamId, const std::string& steamData) {
    json root;
    
    if (authMethod == "login") {
        root["email"] = email;
        root["password"] = password;
    } else {
        root["authMethod"] = authMethod;
        
        if (!username.empty()) root["username"] = username;
        if (!email.empty()) root["email"] = email;
        if (!password.empty()) root["password"] = password;
        if (!steamId.empty()) root["steamId"] = steamId;
        if (!steamData.empty()) {
            try {
                json steamJson = json::parse(steamData);
                root["steamData"] = steamJson;
            } catch (const std::exception& e) {
                // If steamData is invalid JSON, ignore it
            }
        }
    }
    
    return root.dump();
} 