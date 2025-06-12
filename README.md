# Cursor Crisis (Cursor Game)

## ⚠️ Project Status: Paused
This project is currently on hold due to technical limitations in the Cursor AI backend and misalignment with my current goals.

## ❓What It Was
A casual roguelite game using a desktop cursor as the main character. The player would battle through waves of enemies on a digital desktop environment.

## 🔧 Tech Stack

### Backend
- **Node.js with Express.js**: RESTful API server
- **SupaBase**: User data, progress, and achievements storage
- **JWT**: Authentication and session management

### Frontend
- **C++ with SDL2**: Main game engine
- **ImGui (Dear ImGui)**: UI and menu system
- **OpenGL**: Hardware-accelerated graphics rendering

## 🧠 Challenges Faced
- Inability to persist game data due to `401 Unauthorized` backend errors.
- Session state issues (“Cannot save progress - no active session”).
- Game logic worked partially, but core features like leaderboard + score saving were unreliable.

## 🎯 Why It Matters
This was a learning project to explore Cursor AI and game architecture. The idea is shelved for now, but the experience directly informs future projects I’m developing.

### Note: This project was designed to evaluate the capabilities of Cursor AI. The results showed that while Cursor AI performs impressively in frontend development—especially for rapid prototyping and UI generation—it struggles significantly with backend logic, architecture, and debugging.
