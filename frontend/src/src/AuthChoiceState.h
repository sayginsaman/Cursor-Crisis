#pragma once

#include "GameState.h"
#include <string>
#include <memory>

// Forward declarations
class NetworkManager;
class AuthNetworkManager;

enum class AuthMethod {
    NONE,
    STEAM,
    EMAIL,
    LINKED
};

class AuthChoiceState : public GameState {
public:
    AuthChoiceState(Game* game);
    ~AuthChoiceState() override;

    void OnEnter() override;
    void OnExit() override;
    
    void HandleEvent(const SDL_Event& event) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;
    void RenderUI() override;

private:
    // UI State
    AuthMethod m_selectedMethod;
    bool m_isFirstTime;
    std::string m_username;
    std::string m_email;
    std::string m_password;
    std::string m_confirmPassword;
    
    // UI Flags
    bool m_showEmailForm;
    bool m_showEmailLoginForm;
    bool m_showSteamForm;
    bool m_showLinkedForm;
    bool m_isLoading;
    bool m_authSuccessful;
    std::string m_errorMessage;
    std::string m_statusMessage;
    
    // Steam data (would come from Steam API)
    std::string m_steamId;
    std::string m_steamUsername;
    std::string m_steamAvatar;
    
    // Network
    std::unique_ptr<NetworkManager> m_networkManager;
    std::unique_ptr<AuthNetworkManager> m_authNetworkManager;
    
    // Authentication
    std::string m_authToken;
    
    // UI Rendering
    void RenderWelcomeScreen();
    void RenderMethodChoice();
    void RenderEmailForm();
    void RenderEmailLoginForm();
    void RenderSteamForm();
    void RenderLinkedForm();
    void RenderLoadingScreen();
    
    // Actions
    void OnMethodSelected(AuthMethod method);
    void OnSteamAuth();
    void OnEmailRegister();
    void OnEmailLogin();
    void OnLinkedRegister();
    void OnBackToChoice();
    
    // Email validation
    void CheckEmailAndRegister();
    void OnEmailCheckResult(bool exists, const std::string& error);
    
    // Validation
    bool ValidateEmailForm();
    bool ValidateLinkedForm();
    
    // Network callbacks
    void OnAuthSuccess(const std::string& token, const std::string& userData);
    void OnAuthError(const std::string& error);
    
    // Steam integration (placeholder for actual Steam API)
    void InitializeSteam();
    bool IsSteamRunning();
    void GetSteamUserData();
}; 