#include "Renderer.h"
#include <GL/glew.h>
#include <iostream>
#include <cmath>

bool Renderer::Initialize() {
    std::cout << "Renderer initialized" << std::endl;
    
    // Check OpenGL version
    const char* version = (const char*)glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << (version ? version : "Unknown") << std::endl;
    
    // Set up OpenGL state for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set up 2D projection matrix (works with both OpenGL 2.1 and 3.3+)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Use glOrtho for 2D projection - screen coordinates: (0,0) top-left, (1280,720) bottom-right
    glOrtho(0.0, 1280.0, 720.0, 0.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Enable vertex arrays for immediate mode
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    m_initialized = true;
    return true;
}

void Renderer::Shutdown() {
    m_initialized = false;
    std::cout << "Renderer shutdown" << std::endl;
}

void Renderer::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Present() {
    // SDL_GL_SwapWindow is called in Game.cpp
}

void Renderer::DrawRect(float x, float y, float width, float height, float r, float g, float b, float a) {
    if (!m_initialized) return;
    
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Renderer::DrawCircle(float x, float y, float radius, float r, float g, float b, float a) {
    if (!m_initialized) return;
    
    const int segments = 32;
    const float angleStep = 2.0f * M_PI / segments;
    
    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center
    
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();
} 