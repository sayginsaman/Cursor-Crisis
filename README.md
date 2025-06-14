# Cursor Crisis

A casual roguelite game featuring a desktop cursor as the main character. Players battle through waves of desktop-themed enemies on a virtual desktop environment, collecting power-ups, upgrades, and unlocking new skins.

## 🎮 Game Features

- **Cursor-based Gameplay**: Unique mouse-driven character controls
- **Procedurally Generated Waves**: Dynamic enemy spawns for high replayability
- **Upgradeable Skills**: Player progression through collectible upgrades
- **Cosmetic DLC System**: Custom cursors, themes, and seasonal skins
- **Online Leaderboards**: Competitive gameplay with community engagement

## 🛠️ Technology Stack

### Backend
- **Node.js with Express.js**: RESTful API server
- **Supabase**: User data, progress, and achievements storage
- **JWT**: Authentication and session management

### Frontend
- **C++ with SDL2**: Main game engine
- **ImGui (Dear ImGui)**: UI and menu system
- **OpenGL**: Hardware-accelerated graphics rendering

## 📁 Project Structure

```
Desktop-Survivor-Dash/
├── backend/                 # Node.js Express server
│   ├── src/
│   ├── config/
│   └── package.json
├── frontend/               # C++ game client
│   ├── src/
│   ├── assets/
│   ├── libs/
│   └── CMakeLists.txt
├── shared/                 # Shared resources
│   ├── assets/
│   └── configs/
└── docs/                   # Documentation
```

## 🚀 Quick Start

### Backend Setup
1. Navigate to the backend directory: `cd backend`
2. Install dependencies: `npm install`
3. Set up Supabase connection in `.env`
4. Start the server: `npm run dev`

### Frontend Setup
1. Navigate to the frontend directory: `cd frontend`
2. Create build directory: `mkdir build && cd build`
3. Generate makefiles: `cmake ..`
4. Build the game: `make`
5. Run the game: `./desktop-survivor-dash`

## 🎯 Development Roadmap

1. **Phase 1**: Core game mechanics and cursor movement
2. **Phase 2**: Enemy AI and wave generation
3. **Phase 3**: Upgrade system and progression
4. **Phase 4**: UI/UX polishing with ImGui
5. **Phase 5**: Backend integration and leaderboards
6. **Phase 6**: DLC system and monetization
7. **Phase 7**: Steam integration and publishing

## 📋 Requirements

### System Requirements
- **OS**: Windows 10+, macOS 10.15+, Linux Ubuntu 18+
- **RAM**: 4GB minimum, 8GB recommended
- **Graphics**: OpenGL 3.3 compatible
- **Storage**: 2GB available space

### Development Requirements
- **C++ Compiler**: GCC 9+ or Clang 10+ or MSVC 2019+
- **CMake**: 3.16+
- **Node.js**: 16+
- **Supabase**: 5.0+

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🎮 Steam Publishing

This game is designed for Steam distribution with:
- Steam Workshop integration for custom cursors
- Steam Achievements system
- Steam Cloud save synchronization
- Steam Leaderboards integration 

## 📝 Note

This project was created using Cursor AI to evaluate its capabilities. The results showed that while Cursor AI performs exceptionally well in frontend development, it struggles significantly with backend functionality and reliability.


