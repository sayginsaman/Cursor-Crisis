#pragma once

#include "GameState.h"

class MenuState : public GameState {
public:
    MenuState(Game* game);
    ~MenuState() override = default;

    void OnEnter() override;
    void OnExit() override;
    
    void HandleEvent(const SDL_Event& event) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;
    void RenderUI() override;

private:
    bool m_showMainMenu;
    bool m_showSettings;
    bool m_showCredits;
}; 