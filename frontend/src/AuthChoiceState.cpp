#include "AuthChoiceState.h"
#include "HomeState.h"
#include "Game.h"
#include "Renderer.h"
#include "NetworkManager.h"
#include "AuthNetworkManager.h"
#include <SDL2/SDL.h>
#include <imgui.h>
#include <iostream>
#include <ctime>
#include <nlohmann/json.hpp>

AuthChoiceState::AuthChoiceState(Game* game) 
    : GameState(game)
    , m_selectedMethod(AuthMethod::NONE)
    , m_isFirstTime(true)
    , m_showEmailForm(false)
    , m_showEmailLoginForm(false)
    , m_showSteamForm(false)
    , m_showLinkedForm(false)
    , m_isLoading(false)
    , m_authSuccessful(false)
    , m_networkManager(std::make_unique<NetworkManager>())
    , m_authNetworkManager(std::make_unique<AuthNetworkManager>())
{
    // Initialize Steam if available
    InitializeSteam();
}

AuthChoiceState::~AuthChoiceState() = default;

void AuthChoiceState::OnEnter() {
    std::cout << "Entering Auth Choice State" << std::endl;
    
    // Check if this is first time launch
    // In a real implementation, you'd check for saved credentials/tokens
    m_isFirstTime = true;
    
    // Clear any previous state
    m_selectedMethod = AuthMethod::NONE;
    m_showEmailForm = false;
    m_showEmailLoginForm = false;
    m_showSteamForm = false;
    m_showLinkedForm = false;
    m_isLoading = false;
    m_authSuccessful = false;
    m_errorMessage.clear();
    m_statusMessage.clear();
    
    // Clear form data
    m_username.clear();
    m_email.clear();
    m_password.clear();
    m_confirmPassword.clear();
    
    // AUTO-LOGIN FOR TESTING - Use development authentication endpoint
    std::cout << "Auto-logging in with test token for development..." << std::endl;
    
    m_isLoading = true;
    m_statusMessage = "Authenticating for development...";
    
    // Use the development authentication endpoint
    m_authNetworkManager->MakeHttpRequest("/api/auth/dev-login", "POST", "{}", [this](const HttpResponse& response) {
        m_isLoading = false;
        
        if (response.success) {
            try {
                nlohmann::json responseData = nlohmann::json::parse(response.data);
                
                if (responseData.contains("success") && responseData["success"].get<bool>() &&
                    responseData.contains("data") && responseData["data"].contains("token")) {
                    
                    std::string token = responseData["data"]["token"].get<std::string>();
                    std::string username = responseData["data"]["user"]["username"].get<std::string>();
                    
                    OnAuthSuccess(token, "Development authentication successful!");
                } else {
                    OnAuthError("Failed to parse development authentication response");
                }
            } catch (const std::exception& e) {
                OnAuthError("Failed to parse development authentication response: " + std::string(e.what()));
            }
        } else {
            OnAuthError("Development authentication failed: " + response.error);
        }
    });
}

void AuthChoiceState::OnExit() {
    std::cout << "Exiting Auth Choice State" << std::endl;
}

void AuthChoiceState::HandleEvent(const SDL_Event& event) {
    // Handle any specific events if needed
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            if (m_showEmailForm || m_showEmailLoginForm || m_showSteamForm || m_showLinkedForm) {
                OnBackToChoice();
            }
        }
    }
}

void AuthChoiceState::Update(float deltaTime) {
    // Update network manager to process async requests
    if (m_authNetworkManager) {
        m_authNetworkManager->Update();
    }
    
    // Check if authentication was successful and transition to home state
    if (m_authSuccessful) {
        m_authSuccessful = false; // Reset flag
        
        // Create HomeState and pass the auth token
        auto homeState = std::make_unique<HomeState>(m_game);
        homeState->SetAuthToken(m_authToken);
        m_game->ChangeState(std::move(homeState));
    }
}

void AuthChoiceState::Render(Renderer* renderer) {
    // Render background
    renderer->Clear();
    // TODO: Add background color via renderer->DrawRect when needed
}

void AuthChoiceState::RenderUI() {
    if (m_isLoading) {
        RenderLoadingScreen();
        return;
    }
    
    if (m_showEmailForm) {
        RenderEmailForm();
    } else if (m_showEmailLoginForm) {
        RenderEmailLoginForm();
    } else if (m_showSteamForm) {
        RenderSteamForm();
    } else if (m_showLinkedForm) {
        RenderLinkedForm();
    } else {
        RenderWelcomeScreen();
        RenderMethodChoice();
    }
}

void AuthChoiceState::RenderWelcomeScreen() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Center the welcome dialog
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.3f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_Always);
    
    ImGui::Begin("Welcome to Desktop Survivor Dash!", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    // Title
    ImGui::PushFont(io.Fonts->Fonts[0]); // Assume first font is larger
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Welcome, Survivor!").x) * 0.5f);
    ImGui::Text("Welcome, Survivor!");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Description
    ImGui::TextWrapped("To start your survival journey, please choose how you'd like to create your account. "
                      "Your choice will determine how you sign in to track your progress and compete on leaderboards.");
    
    ImGui::Spacing();
    
    // Benefits text
    ImGui::Text("Your account will track:");
    ImGui::BulletText("Game statistics and personal bests");
    ImGui::BulletText("Skill progression and upgrades");
    ImGui::BulletText("Achievements and unlocks");
    ImGui::BulletText("Leaderboard rankings");
    
    ImGui::End();
}

void AuthChoiceState::RenderMethodChoice() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Center the choice dialog
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.7f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 300), ImGuiCond_Always);
    
    ImGui::Begin("Choose Your Account Type", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    // Steam Option
    ImGui::BeginGroup();
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        
        if (ImGui::Button("🎮 Use Steam Account", ImVec2(200, 80))) {
            OnMethodSelected(AuthMethod::STEAM);
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::Text("Quick & Easy");
        ImGui::TextWrapped("• Instant login with Steam\n• Use your Steam profile\n• Steam achievements");
        
        if (!IsSteamRunning()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
            ImGui::Text("⚠ Steam not detected");
            ImGui::PopStyleColor();
        }
    }
    ImGui::EndGroup();
    
    ImGui::SameLine(250);
    
    // Email Option
    ImGui::BeginGroup();
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.3f, 0.1f, 1.0f));
        
        if (ImGui::Button("📧 Create Game Account", ImVec2(200, 50))) {
            OnMethodSelected(AuthMethod::EMAIL);
        }
        
        ImGui::PopStyleColor(3);
        
        // Login button for existing users
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.4f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.2f, 0.0f, 1.0f));
        
        if (ImGui::Button("🔑 Login to Game Account", ImVec2(200, 25))) {
            m_selectedMethod = AuthMethod::EMAIL;
            m_showEmailLoginForm = true;
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::Text("Full Control");
        ImGui::TextWrapped("• Email & password\n• Platform independent\n• Your data, your control");
    }
    ImGui::EndGroup();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Advanced option
    ImGui::Separator();
    ImGui::Text("Advanced Option:");
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.3f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.7f, 1.0f));
    
    if (ImGui::Button("🔗 Link Both Accounts", ImVec2(300, 40))) {
        OnMethodSelected(AuthMethod::LINKED);
    }
    
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::TextWrapped("Best of both worlds - use Steam OR email to login");
    
    // Error message
    if (!m_errorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::Text("Error: %s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    // Development/Testing Option
    ImGui::Separator();
    ImGui::Text("Development Mode:");
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
    
    if (ImGui::Button("🚀 Play Without Account (Test Mode)", ImVec2(400, 30))) {
        // Use a test token for development
        OnAuthSuccess("test_token_for_development_" + std::to_string(time(nullptr)), "Test user");
    }
    
    ImGui::PopStyleColor(3);
    
    ImGui::TextWrapped("Skip authentication for testing purposes");
    
    ImGui::End();
}

void AuthChoiceState::RenderEmailForm() {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Always);
    
    ImGui::Begin("Create Game Account", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Create your Desktop Survivor Dash account");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Username field
    ImGui::Text("Username:");
    static char username_buf[64] = "";
    if (ImGui::InputText("##username", username_buf, sizeof(username_buf))) {
        m_username = username_buf;
    }
    
    ImGui::Spacing();
    
    // Email field
    ImGui::Text("Email:");
    static char email_buf[128] = "";
    if (ImGui::InputText("##email", email_buf, sizeof(email_buf))) {
        m_email = email_buf;
    }
    
    ImGui::Spacing();
    
    // Password field
    ImGui::Text("Password:");
    static char password_buf[128] = "";
    if (ImGui::InputText("##password", password_buf, sizeof(password_buf), ImGuiInputTextFlags_Password)) {
        m_password = password_buf;
    }
    
    ImGui::Spacing();
    
    // Confirm password field
    ImGui::Text("Confirm Password:");
    static char confirm_buf[128] = "";
    if (ImGui::InputText("##confirm", confirm_buf, sizeof(confirm_buf), ImGuiInputTextFlags_Password)) {
        m_confirmPassword = confirm_buf;
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Buttons
    if (ImGui::Button("Create Account", ImVec2(150, 30))) {
        OnEmailRegister();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Back", ImVec2(100, 30))) {
        OnBackToChoice();
    }
    
    ImGui::Spacing();
    
    // Switch to login
    if (ImGui::Button("Have an account? Login", ImVec2(200, 25))) {
        m_showEmailForm = false;
        m_showEmailLoginForm = true;
    }
    
    // Error/status messages
    if (!m_errorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    if (!m_statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::TextWrapped("Status: %s", m_statusMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
}

void AuthChoiceState::RenderSteamForm() {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_Always);
    
    ImGui::Begin("Steam Account Setup", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Link your Steam account to Desktop Survivor Dash");
    ImGui::Separator();
    ImGui::Spacing();
    
    if (IsSteamRunning()) {
        ImGui::Text("✅ Steam detected!");
        ImGui::Spacing();
        
        // Display Steam user info (mock data for now)
        ImGui::Text("Steam Username: %s", m_steamUsername.c_str());
        ImGui::Text("Steam ID: %s", m_steamId.c_str());
        
        ImGui::Spacing();
        ImGui::TextWrapped("Your Steam profile will be used for your game account. "
                          "You can always add an email later if you want to play on other platforms.");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Username override
        ImGui::Text("Game Username (optional):");
        static char username_buf[64] = "";
        if (ImGui::InputText("##username", username_buf, sizeof(username_buf))) {
            m_username = username_buf;
        }
        ImGui::TextWrapped("Leave empty to use your Steam username");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        if (ImGui::Button("Create Steam Account", ImVec2(200, 30))) {
            OnSteamAuth();
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
        ImGui::Text("⚠ Steam not running or not detected");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::TextWrapped("Please make sure Steam is running and you're logged in, then try again.");
        
        ImGui::Spacing();
        
        if (ImGui::Button("Retry", ImVec2(100, 30))) {
            InitializeSteam();
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Back", ImVec2(100, 30))) {
        OnBackToChoice();
    }
    
    // Error messages
    if (!m_errorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
}

void AuthChoiceState::RenderLinkedForm() {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(550, 500), ImGuiCond_Always);
    
    ImGui::Begin("Link Steam + Email Account", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Create a linked account for maximum flexibility");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextWrapped("This creates an account you can access with EITHER Steam OR email/password. "
                      "Perfect if you want to play on multiple platforms!");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Steam section
    ImGui::Text("Steam Account:");
    if (IsSteamRunning()) {
        ImGui::Text("✅ Steam: %s (%s)", m_steamUsername.c_str(), m_steamId.c_str());
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
        ImGui::Text("⚠ Steam not detected");
        ImGui::PopStyleColor();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Email section
    ImGui::Text("Email Account Details:");
    
    // Username
    ImGui::Text("Username:");
    static char username_buf[64] = "";
    if (ImGui::InputText("##username", username_buf, sizeof(username_buf))) {
        m_username = username_buf;
    }
    
    // Email
    ImGui::Text("Email:");
    static char email_buf[128] = "";
    if (ImGui::InputText("##email", email_buf, sizeof(email_buf))) {
        m_email = email_buf;
    }
    
    // Password
    ImGui::Text("Password:");
    static char password_buf[128] = "";
    if (ImGui::InputText("##password", password_buf, sizeof(password_buf), ImGuiInputTextFlags_Password)) {
        m_password = password_buf;
    }
    
    // Confirm password
    ImGui::Text("Confirm Password:");
    static char confirm_buf[128] = "";
    if (ImGui::InputText("##confirm", confirm_buf, sizeof(confirm_buf), ImGuiInputTextFlags_Password)) {
        m_confirmPassword = confirm_buf;
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Buttons
    bool canCreate = IsSteamRunning() && !m_username.empty() && !m_email.empty() && !m_password.empty();
    
    if (!canCreate) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }
    
    if (ImGui::Button("Create Linked Account", ImVec2(200, 30)) && canCreate) {
        OnLinkedRegister();
    }
    
    if (!canCreate) {
        ImGui::PopStyleVar();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Back", ImVec2(100, 30))) {
        OnBackToChoice();
    }
    
    // Requirements
    if (!canCreate) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
        ImGui::Text("Requirements: Steam running + all fields filled");
        ImGui::PopStyleColor();
    }
    
    // Error messages
    if (!m_errorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
}

void AuthChoiceState::RenderLoadingScreen() {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
    
    ImGui::Begin("Creating Account...", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Creating Account...").x) * 0.5f);
    ImGui::Text("Creating Account...");
    
    ImGui::Spacing();
    
    // Simple loading animation
    static float loadingTime = 0.0f;
    loadingTime += ImGui::GetIO().DeltaTime;
    
    std::string dots = "";
    int numDots = (int)(loadingTime * 2) % 4;
    for (int i = 0; i < numDots; i++) {
        dots += ".";
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Please wait").x) * 0.5f);
    ImGui::Text("Please wait%s", dots.c_str());
    
    if (!m_statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(m_statusMessage.c_str()).x) * 0.5f);
        ImGui::Text("%s", m_statusMessage.c_str());
    }
    
    ImGui::End();
}

// Action handlers
void AuthChoiceState::OnMethodSelected(AuthMethod method) {
    m_selectedMethod = method;
    m_errorMessage.clear();
    
    switch (method) {
        case AuthMethod::STEAM:
            m_showSteamForm = true;
            GetSteamUserData();
            break;
        case AuthMethod::EMAIL:
            m_showEmailForm = true;
            break;
        case AuthMethod::LINKED:
            m_showLinkedForm = true;
            GetSteamUserData();
            break;
        default:
            break;
    }
}

void AuthChoiceState::OnSteamAuth() {
    if (!IsSteamRunning()) {
        m_errorMessage = "Steam is not running. Please start Steam and try again.";
        return;
    }
    
    GetSteamUserData();
    
    m_isLoading = true;
    m_statusMessage = "Authenticating with Steam...";
    m_errorMessage.clear();
    
    // Check if this is a new Steam user or existing one
    m_authNetworkManager->LoginSteamUser(m_steamId, [this](const AuthResponse& response) {
        if (response.success) {
            // Existing Steam user
            OnAuthSuccess(response.token, "Steam login successful!");
        } else {
            // New Steam user, create account
            m_authNetworkManager->CreateSteamUser(m_steamId, m_steamUsername, m_steamAvatar, [this](const AuthResponse& createResponse) {
                m_isLoading = false;
                
                if (createResponse.success) {
                    OnAuthSuccess(createResponse.token, "Steam account created successfully!");
                } else {
                    OnAuthError(createResponse.error);
                }
            });
        }
    });
}

void AuthChoiceState::OnEmailRegister() {
    // First check if email already exists
    if (ValidateEmailForm()) {
        CheckEmailAndRegister();
    }
}

void AuthChoiceState::OnLinkedRegister() {
    if (!ValidateLinkedForm()) {
        return;
    }
    
    if (!IsSteamRunning()) {
        m_errorMessage = "Steam is not running. Please start Steam to link your account.";
        return;
    }
    
    GetSteamUserData();
    
    // First check if email already exists
    m_isLoading = true;
    m_statusMessage = "Checking email...";
    m_errorMessage.clear();
    
    m_authNetworkManager->CheckEmailExists(m_email, [this](bool exists, const std::string& error) {
        if (!error.empty()) {
            m_isLoading = false;
            m_errorMessage = "Network error: " + error;
            return;
        }
        
        if (exists) {
            m_isLoading = false;
            m_errorMessage = "Email is already in use. Please use a different email.";
            return;
        }
        
        // Email is available, create linked account
        m_statusMessage = "Creating linked account...";
        
        m_authNetworkManager->CreateLinkedUser(m_username, m_email, m_password, m_steamId, m_steamAvatar, [this](const AuthResponse& response) {
            m_isLoading = false;
            
            if (response.success) {
                OnAuthSuccess(response.token, "Linked account created successfully!");
            } else {
                OnAuthError(response.error);
            }
        });
    });
}

void AuthChoiceState::OnBackToChoice() {
    m_selectedMethod = AuthMethod::NONE;
    m_showEmailForm = false;
    m_showEmailLoginForm = false;
    m_showSteamForm = false;
    m_showLinkedForm = false;
    m_errorMessage.clear();
    m_statusMessage.clear();
}

// Validation
bool AuthChoiceState::ValidateEmailForm() {
    if (m_username.length() < 3) {
        m_errorMessage = "Username must be at least 3 characters long";
        return false;
    }
    
    if (m_email.empty() || m_email.find('@') == std::string::npos) {
        m_errorMessage = "Please enter a valid email address";
        return false;
    }
    
    if (m_password.length() < 6) {
        m_errorMessage = "Password must be at least 6 characters long";
        return false;
    }
    
    if (m_password != m_confirmPassword) {
        m_errorMessage = "Passwords do not match";
        return false;
    }
    
    return true;
}

bool AuthChoiceState::ValidateLinkedForm() {
    if (!IsSteamRunning()) {
        m_errorMessage = "Steam is required for linked accounts";
        return false;
    }
    
    return ValidateEmailForm();
}

// Network callbacks
void AuthChoiceState::OnAuthSuccess(const std::string& token, const std::string& userData) {
    m_isLoading = false;
    m_statusMessage = "Success! Logging in...";
    
    // Store token securely
    m_authToken = token;
    std::cout << "Authentication successful! Token received." << std::endl;
    
    // TODO: Parse user data
    
    // Set flag to transition to main game on next update (thread-safe)
    m_authSuccessful = true;
}

void AuthChoiceState::OnAuthError(const std::string& error) {
    m_isLoading = false;
    m_errorMessage = error;
    m_statusMessage.clear();
}

// Steam integration (placeholder)
void AuthChoiceState::InitializeSteam() {
    // TODO: Initialize Steam API
    // For now, simulate Steam detection
    
    // Clear any mock data - will be populated by real Steam API
    m_steamId.clear();
    m_steamUsername.clear();
    m_steamAvatar.clear();
}

bool AuthChoiceState::IsSteamRunning() {
    // TODO: Check if Steam is actually running via Steam API
    // For now, return true for demo purposes
    return true;
}

void AuthChoiceState::GetSteamUserData() {
    if (IsSteamRunning()) {
        // TODO: Get actual Steam user data from Steam API
        // For now, clear data until real implementation
        m_steamId.clear();
        m_steamUsername.clear();
        m_steamAvatar.clear();
    }
}

void AuthChoiceState::RenderEmailLoginForm() {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 300), ImGuiCond_Always);
    
    ImGui::Begin("Login to Game Account", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Welcome back, Cursor Warrior!");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Email field
    ImGui::Text("Email:");
    static char email_buf[128] = "";
    if (ImGui::InputText("##email", email_buf, sizeof(email_buf))) {
        m_email = email_buf;
    }
    
    ImGui::Spacing();
    
    // Password field
    ImGui::Text("Password:");
    static char password_buf[128] = "";
    if (ImGui::InputText("##password", password_buf, sizeof(password_buf), ImGuiInputTextFlags_Password)) {
        m_password = password_buf;
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Buttons
    if (ImGui::Button("Login", ImVec2(150, 30))) {
        OnEmailLogin();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Back", ImVec2(100, 30))) {
        OnBackToChoice();
    }
    
    ImGui::Spacing();
    
    // Switch to signup
    if (ImGui::Button("Don't have an account? Sign up", ImVec2(250, 25))) {
        m_showEmailLoginForm = false;
        m_showEmailForm = true;
    }
    
    // Error/status messages
    if (!m_errorMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("Error: %s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    if (!m_statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        ImGui::TextWrapped("Status: %s", m_statusMessage.c_str());
        ImGui::PopStyleColor();
    }
    
    ImGui::End();
}

void AuthChoiceState::OnEmailLogin() {
    // Validate input
    if (m_email.empty() || m_email.find('@') == std::string::npos) {
        m_errorMessage = "Please enter a valid email address";
        return;
    }
    
    if (m_password.empty()) {
        m_errorMessage = "Please enter your password";
        return;
    }
    
    m_isLoading = true;
    m_statusMessage = "Logging in...";
    m_errorMessage.clear();
    
    m_authNetworkManager->LoginEmailUser(m_email, m_password, [this](const AuthResponse& response) {
        m_isLoading = false;
        
        if (response.success) {
            OnAuthSuccess(response.token, "Login successful!");
        } else {
            OnAuthError(response.error);
        }
    });
}

void AuthChoiceState::CheckEmailAndRegister() {
    m_isLoading = true;
    m_statusMessage = "Checking email...";
    m_errorMessage.clear();
    
    m_authNetworkManager->CheckEmailExists(m_email, [this](bool exists, const std::string& error) {
        OnEmailCheckResult(exists, error);
    });
}

void AuthChoiceState::OnEmailCheckResult(bool exists, const std::string& error) {
    m_isLoading = false;
    
    if (!error.empty()) {
        m_errorMessage = "Network error: " + error;
        m_statusMessage.clear();
        return;
    }
    
    if (exists) {
        m_errorMessage = "Email is already in use. Please login or use a different email.";
        m_statusMessage.clear();
        return;
    }
    
    // Email is available, proceed with registration
    m_isLoading = true;
    m_statusMessage = "Creating account...";
    m_errorMessage.clear();
    
    m_authNetworkManager->RegisterEmailUser(m_username, m_email, m_password, [this](const AuthResponse& response) {
        m_isLoading = false;
        
        if (response.success) {
            OnAuthSuccess(response.token, "Email account created successfully!");
        } else {
            OnAuthError(response.error);
        }
    });
} 