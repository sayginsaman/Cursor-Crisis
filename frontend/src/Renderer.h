#pragma once

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;

    bool Initialize();
    void Shutdown();
    
    void Clear();
    void Present();
    
    // Basic rendering methods
    void DrawRect(float x, float y, float width, float height, float r, float g, float b, float a = 1.0f);
    void DrawCircle(float x, float y, float radius, float r, float g, float b, float a = 1.0f);

private:
    bool m_initialized = false;
}; 