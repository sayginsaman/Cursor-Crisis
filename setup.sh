#!/bin/bash

echo "🎮 Desktop Survivor Dash - Setup Script"
echo "======================================="

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    OS="windows"
else
    echo "❌ Unsupported operating system: $OSTYPE"
    exit 1
fi

echo "🔍 Detected OS: $OS"

# Install dependencies based on OS
install_dependencies() {
    echo "📦 Installing dependencies..."
    
    case $OS in
        "linux")
            # Ubuntu/Debian
            if command -v apt-get &> /dev/null; then
                sudo apt-get update
                sudo apt-get install -y \
                    build-essential \
                    cmake \
                    pkg-config \
                    libsdl2-dev \
                    libsdl2-mixer-dev \
                    libsdl2-image-dev \
                    libsdl2-ttf-dev \
                    libglew-dev \
                    libglm-dev \
                    libcurl4-openssl-dev \
                    git
            # Fedora/RHEL
            elif command -v dnf &> /dev/null; then
                sudo dnf install -y \
                    gcc-c++ \
                    cmake \
                    SDL2-devel \
                    SDL2_mixer-devel \
                    SDL2_image-devel \
                    SDL2_ttf-devel \
                    glew-devel \
                    glm-devel \
                    libcurl-devel \
                    git
            # Arch Linux
            elif command -v pacman &> /dev/null; then
                sudo pacman -S --noconfirm \
                    base-devel \
                    cmake \
                    sdl2 \
                    sdl2_mixer \
                    sdl2_image \
                    sdl2_ttf \
                    glew \
                    glm \
                    curl \
                    git
            else
                echo "❌ Unknown Linux distribution. Please install dependencies manually."
                exit 1
            fi
            ;;
        "macos")
            # Check if Homebrew is installed
            if ! command -v brew &> /dev/null; then
                echo "📥 Installing Homebrew..."
                /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
            fi
            
            # Install dependencies via Homebrew
            brew install \
                cmake \
                sdl2 \
                sdl2_mixer \
                sdl2_image \
                sdl2_ttf \
                glew \
                glm \
                curl \
                git
            ;;
        "windows")
            echo "🪟 For Windows, please use vcpkg or manually install dependencies:"
            echo "  - SDL2"
            echo "  - GLEW"
            echo "  - GLM"
            echo "  - libcurl"
            echo "  - CMake"
            echo "  - Visual Studio 2019+ or MinGW"
            echo ""
            echo "Example with vcpkg:"
            echo "  git clone https://github.com/Microsoft/vcpkg.git"
            echo "  .\\vcpkg\\bootstrap-vcpkg.bat"
            echo "  .\\vcpkg\\vcpkg install sdl2 glew glm curl"
            exit 0
            ;;
    esac
}

# Clone and setup ImGui
setup_imgui() {
    echo "🖼️  Setting up ImGui..."
    
    cd frontend
    mkdir -p libs
    cd libs
    
    if [ ! -d "imgui" ]; then
        git clone https://github.com/ocornut/imgui.git
        echo "✅ ImGui cloned successfully"
    else
        echo "📁 ImGui already exists, skipping..."
    fi
    
    cd ../..
}

# Clone and setup nlohmann/json
setup_json() {
    echo "📄 Setting up JSON library..."
    
    cd frontend/libs
    
    if [ ! -d "json" ]; then
        git clone https://github.com/nlohmann/json.git
        echo "✅ JSON library cloned successfully"
    else
        echo "📁 JSON library already exists, skipping..."
    fi
    
    cd ../..
}

# Setup MongoDB (for backend)
setup_mongodb() {
    echo "🗄️  Setting up MongoDB..."
    
    case $OS in
        "linux")
            # Install MongoDB
            curl -fsSL https://pgp.mongodb.com/server-6.0.asc | sudo gpg -o /usr/share/keyrings/mongodb-server-6.0.gpg --dearmor
            echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-6.0.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/6.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-6.0.list
            sudo apt-get update
            sudo apt-get install -y mongodb-org
            sudo systemctl start mongod
            sudo systemctl enable mongod
            ;;
        "macos")
            brew tap mongodb/brew
            brew install mongodb-community
            brew services start mongodb/brew/mongodb-community
            ;;
    esac
}

# Setup Node.js (for backend)
setup_nodejs() {
    echo "🟢 Setting up Node.js..."
    
    if ! command -v node &> /dev/null; then
        case $OS in
            "linux")
                curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
                sudo apt-get install -y nodejs
                ;;
            "macos")
                brew install node
                ;;
        esac
    else
        echo "✅ Node.js already installed: $(node --version)"
    fi
    
    # Install backend dependencies
    if [ -d "backend" ]; then
        echo "📦 Installing backend dependencies..."
        cd backend
        npm install
        cd ..
        echo "✅ Backend dependencies installed"
    fi
}

# Build frontend
build_frontend() {
    echo "🔨 Building frontend..."
    
    cd frontend
    mkdir -p build
    cd build
    
    cmake ..
    if [ $? -eq 0 ]; then
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
        if [ $? -eq 0 ]; then
            echo "✅ Frontend built successfully!"
        else
            echo "❌ Frontend build failed"
            exit 1
        fi
    else
        echo "❌ CMake configuration failed"
        exit 1
    fi
    
    cd ../..
}

# Main setup process
main() {
    echo "🚀 Starting setup process..."
    
    # Install system dependencies
    install_dependencies
    
    # Setup libraries
    setup_imgui
    setup_json
    
    # Setup backend
    setup_nodejs
    setup_mongodb
    
    # Build frontend
    build_frontend
    
    echo ""
    echo "🎉 Setup complete!"
    echo ""
    echo "📖 Next steps:"
    echo "1. Backend: cd backend && npm run dev"
    echo "2. Frontend: cd frontend/build && ./DesktopSurvivorDash"
    echo ""
    echo "🌐 Backend will run on: http://localhost:3000"
    echo "🎮 Game client will connect to the backend API"
    echo ""
    echo "Happy gaming! 🎮✨"
}

# Check if running with --help
if [[ $1 == "--help" || $1 == "-h" ]]; then
    echo "Desktop Survivor Dash Setup Script"
    echo ""
    echo "Usage: ./setup.sh [options]"
    echo ""
    echo "Options:"
    echo "  --help, -h     Show this help message"
    echo "  --frontend     Setup frontend only"
    echo "  --backend      Setup backend only"
    echo ""
    echo "This script will:"
    echo "  1. Install system dependencies (SDL2, OpenGL, etc.)"
    echo "  2. Clone and setup ImGui and JSON libraries"
    echo "  3. Install Node.js and MongoDB"
    echo "  4. Build the C++ game client"
    echo "  5. Setup the Node.js backend server"
    exit 0
fi

# Check for specific setup options
if [[ $1 == "--frontend" ]]; then
    install_dependencies
    setup_imgui
    setup_json
    build_frontend
elif [[ $1 == "--backend" ]]; then
    setup_nodejs
    setup_mongodb
elif [[ $1 == "--dependencies" ]]; then
    install_dependencies
else
    main
fi 