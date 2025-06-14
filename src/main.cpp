#include <iostream>
#include <stdexcept>
#include "Game.h"

int main(int argc, char* argv[]) {
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;
    
    try {
        // Create and run the game
        Game game;
        
        if (!game.Initialize()) {
            std::cerr << "Failed to initialize game!" << std::endl;
            return -1;
        }
        
        // Run the main game loop
        game.Run();
        
        // Clean up
        game.Shutdown();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Game crashed with exception: " << e.what() << std::endl;
        return -1;
    }
    catch (...) {
        std::cerr << "Game crashed with unknown exception!" << std::endl;
        return -1;
    }
} 