# Desktop Survivor Dash - Development Guide

## 🎯 Project Overview

Desktop Survivor Dash is a casual roguelite game featuring a desktop cursor as the main character. Players battle through waves of desktop-themed enemies on a virtual desktop environment, collecting power-ups, upgrades, and unlocking new skins.

### Key Features
- **Unique Cursor-based Gameplay**: Mouse-driven character controls
- **Procedural Wave Generation**: Dynamic enemy spawns for high replayability
- **Progressive Upgrade System**: Collectible upgrades and customizable abilities
- **Monetization via DLC**: Custom cursors, themes, and seasonal skins
- **Online Features**: Leaderboards, achievements, and user profiles
- **Steam Integration**: Workshop support, achievements, and cloud saves

## 🏗️ Architecture

### Backend (Node.js + Express + MongoDB)
- **RESTful API**: User authentication, game statistics, leaderboards
- **Database**: MongoDB for user data, game sessions, and DLC items
- **Authentication**: JWT-based user sessions
- **Monetization**: DLC purchase system with payment integration

### Frontend (C++ + SDL2 + OpenGL)
- **Game Engine**: Custom C++ engine with SDL2 for windowing and input
- **Graphics**: OpenGL for hardware-accelerated rendering
- **UI System**: ImGui for menus and in-game interfaces
- **Network**: HTTP client for backend API communication

## 📁 Project Structure

```
Desktop-Survivor-Dash/
├── backend/                    # Node.js Express API server
│   ├── src/
│   │   ├── middleware/         # Authentication, error handling
│   │   ├── models/            # MongoDB schemas (User, GameSession, DLCItem)
│   │   ├── routes/            # API endpoints (auth, game, leaderboard, dlc)
│   │   └── server.js          # Main server file
│   ├── scripts/               # Database seeding and utilities
│   └── package.json
├── frontend/                   # C++ game client
│   ├── src/                   # Game source code
│   │   ├── UI/                # ImGui interface components
│   │   ├── Utils/             # Utility classes and helpers
│   │   ├── Network/           # HTTP client for API communication
│   │   ├── Game.cpp/.h        # Main game class
│   │   └── main.cpp           # Entry point
│   ├── libs/                  # Third-party libraries (ImGui, JSON)
│   ├── assets/                # Game assets (textures, sounds)
│   └── CMakeLists.txt         # Build configuration
├── shared/                    # Shared resources
│   └── configs/               # Game configuration files
├── setup.sh                  # Automated setup script
└── README.md
```

## 🚀 Getting Started

### Prerequisites
- **C++ Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: 3.16+
- **Node.js**: 16+
- **MongoDB**: 5.0+
- **Git**: For cloning dependencies

### Quick Setup
```bash
# Clone the repository
git clone <your-repo-url>
cd desktop-survivor-dash

# Run the setup script
chmod +x setup.sh
./setup.sh

# Or setup components individually
./setup.sh --backend    # Backend only
./setup.sh --frontend   # Frontend only
```

### Manual Setup

#### Backend Setup
```bash
cd backend
npm install
cp env.example .env
# Edit .env with your configuration
npm run dev
```

#### Frontend Setup
```bash
cd frontend
mkdir -p libs build

# Clone dependencies
cd libs
git clone https://github.com/ocornut/imgui.git
git clone https://github.com/nlohmann/json.git
cd ..

# Build the game
cd build
cmake ..
make -j$(nproc)
./DesktopSurvivorDash
```

## 🎮 Game Development

### Core Game Loop
1. **Initialization**: SDL2, OpenGL, ImGui setup
2. **Game States**: Menu → Play → GameOver → Leaderboard
3. **Update Cycle**: Input → Game Logic → Rendering
4. **Network Sync**: Score submission, leaderboard updates

### Key Systems

#### Player System
- Cursor-based movement and controls
- Health, damage, and upgrade management
- Experience and leveling system

#### Enemy System
- Procedural spawning based on wave progression
- AI behaviors (chase, patrol, special attacks)
- Scaling difficulty and variety

#### Upgrade System
- Collectible power-ups during gameplay
- Persistent upgrades between sessions
- Skill tree progression

#### DLC System
- Cosmetic cursor skins and themes
- Seasonal and limited-time content
- Steam Workshop integration

## 💰 Monetization Strategy

### DLC Content Types
1. **Cursor Skins**: Animated cursors with different themes
2. **Desktop Themes**: Background environments and effects
3. **Seasonal Packs**: Holiday and event-specific content
4. **Premium Upgrades**: Exclusive gameplay enhancements

### Pricing Tiers
- **Common**: $0.99 - $1.49
- **Rare**: $1.99 - $2.99
- **Epic**: $3.49 - $4.99
- **Legendary**: $5.99 - $9.99

### Revenue Streams
1. **Base Game Sales**: One-time purchase on Steam
2. **DLC Sales**: Ongoing cosmetic content
3. **Season Passes**: Bundled content releases
4. **Steam Workshop**: Community-created content (revenue share)

## 🔧 API Documentation

### Authentication Endpoints
```
POST /api/auth/register    # User registration
POST /api/auth/login       # User login
GET  /api/auth/me          # Get current user
POST /api/auth/refresh     # Refresh JWT token
```

### Game Endpoints
```
POST /api/game/session/start    # Start game session
POST /api/game/session/end      # Submit game results
GET  /api/game/stats            # Get user statistics
GET  /api/game/achievements     # Get achievements
```

### Leaderboard Endpoints
```
GET /api/leaderboard/scores     # Top scores
GET /api/leaderboard/survival   # Longest survival times
GET /api/leaderboard/kills      # Most kills
GET /api/leaderboard/recent     # Recent high scores
```

### DLC Endpoints
```
GET  /api/dlc/items        # Browse DLC items
GET  /api/dlc/featured     # Featured items
POST /api/dlc/purchase     # Purchase DLC item
GET  /api/dlc/owned        # User's owned items
```

## 🎨 Asset Pipeline

### Cursor Assets
- **Format**: PNG with alpha channel
- **Resolution**: 32x32 or 64x64 pixels
- **Animation**: Frame-based sprite sheets
- **Naming**: `cursor-{name}-{frame}.png`

### Theme Assets
- **Background**: 1920x1080 base resolution
- **Effects**: Particle systems and shaders
- **Audio**: Ambient soundtracks (OGG format)

### Asset Organization
```
assets/
├── cursors/
│   ├── default/
│   ├── golden-arrow/
│   └── neon-blade/
├── themes/
│   ├── default/
│   ├── cyberpunk-city/
│   └── forest-sanctuary/
├── sounds/
└── music/
```

## 🧪 Testing Strategy

### Backend Testing
```bash
cd backend
npm test                    # Run all tests
npm run test:unit          # Unit tests only
npm run test:integration   # Integration tests
```

### Frontend Testing
- **Unit Tests**: Core game logic testing
- **Integration Tests**: API communication testing
- **Performance Tests**: Frame rate and memory usage
- **Platform Tests**: Windows, macOS, Linux compatibility

## 📦 Deployment

### Backend Deployment
- **Platform**: AWS, Heroku, or DigitalOcean
- **Database**: MongoDB Atlas (cloud)
- **CDN**: CloudFlare for asset delivery
- **Monitoring**: Application performance monitoring

### Frontend Distribution
- **Primary**: Steam Store
- **Secondary**: Itch.io, Epic Games Store
- **Packaging**: Platform-specific installers
- **Updates**: Steam's automatic update system

## 🔄 Development Workflow

### Version Control
```bash
# Feature development
git checkout -b feature/new-cursor-system
git commit -m "Add new cursor animation system"
git push origin feature/new-cursor-system

# Create pull request for review
```

### Release Process
1. **Development**: Feature branches → dev branch
2. **Testing**: Automated tests + manual QA
3. **Staging**: Deploy to staging environment
4. **Production**: Release to Steam + backend deployment

### Continuous Integration
- **Build**: Automated builds for all platforms
- **Test**: Run test suites on every commit
- **Deploy**: Automatic deployment to staging

## 📊 Analytics and Metrics

### Key Performance Indicators (KPIs)
- **Player Retention**: Daily/Weekly/Monthly active users
- **Session Length**: Average gameplay duration
- **Monetization**: DLC conversion rates and revenue
- **Engagement**: Leaderboard participation

### Analytics Tools
- **Game Analytics**: Player behavior tracking
- **Steam Analytics**: Sales and user metrics
- **Custom Metrics**: API usage and performance

## 🛡️ Security Considerations

### Backend Security
- **Authentication**: JWT with secure secret rotation
- **Rate Limiting**: Prevent API abuse
- **Input Validation**: Sanitize all user inputs
- **HTTPS**: Encrypted communication

### Client Security
- **Anti-cheat**: Server-side score validation
- **Asset Protection**: Encrypted asset files
- **Update Security**: Signed update packages

## 🚀 Future Roadmap

### Phase 1: Core Game (Months 1-3)
- ✅ Basic gameplay mechanics
- ✅ Backend API and database
- ✅ User authentication and profiles
- ✅ Basic DLC system

### Phase 2: Content & Polish (Months 4-6)
- [ ] Advanced enemy AI and behaviors
- [ ] Comprehensive upgrade system
- [ ] Steam integration (achievements, workshop)
- [ ] Performance optimization

### Phase 3: Launch & Growth (Months 7-9)
- [ ] Steam Early Access release
- [ ] Community feedback integration
- [ ] Additional DLC content
- [ ] Marketing and user acquisition

### Phase 4: Post-Launch (Months 10+)
- [ ] Regular content updates
- [ ] Community events and competitions
- [ ] Platform expansion (consoles)
- [ ] Sequel planning

## 📞 Support and Community

### Development Team
- **Lead Developer**: Game architecture and core systems
- **Backend Developer**: API and database management
- **Artist**: Cursor designs and theme creation
- **QA Tester**: Quality assurance and bug testing

### Community Channels
- **Discord**: Real-time community interaction
- **Steam Forums**: Official game discussions
- **Reddit**: Community-driven content
- **Twitter**: Development updates and announcements

---

**Happy coding! 🎮✨** 