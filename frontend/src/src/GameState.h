#pragma once

// Forward declarations
union SDL_Event;
class Game;
class Renderer;

class GameState {
public:
    GameState(Game* game) : m_game(game) {}
    virtual ~GameState() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnPause() {}
    virtual void OnResume() {}
    
    virtual void HandleEvent(const SDL_Event& event) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(Renderer* renderer) = 0;
    virtual void RenderUI() = 0;

protected:
    Game* m_game;
}; 