#!/bin/bash

# Desktop Survivor Dash - Game Launcher
echo "🎮 Starting Desktop Survivor Dash..."

# Check if build directory exists
if [ ! -d "frontend/build" ]; then
    echo "❌ Build directory not found. Please build the game first:"
    echo "   cd frontend/build && cmake .. && make"
    exit 1
fi

# Check if executable exists
if [ ! -f "frontend/build/DesktopSurvivorDash.app/Contents/MacOS/DesktopSurvivorDash" ]; then
    echo "❌ Game executable not found. Please build the game first:"
    echo "   cd frontend/build && make"
    exit 1
fi

# Change to build directory and run the game
cd frontend/build
echo "🚀 Launching game..."
./DesktopSurvivorDash.app/Contents/MacOS/DesktopSurvivorDash

echo "🎮 Game session ended." 