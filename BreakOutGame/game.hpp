//
//  game.hpp
//  BreakOutGame
//
//  Created by Daniel.Lin on 2019/7/14.
//  Copyright © 2019 Daniel.Lin. All rights reserved.
//

#ifndef game_hpp
#define game_hpp

#include <stdio.h>

#include "resource_manager.hpp"
#include "sprite_renderer.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
    
public:
    // Game state
    GameState              State;
    GLboolean              Keys[1024];
    GLuint                 Width, Height;
    // Constructor/Destructor
    Game(GLuint width, GLuint height);
    ~Game();
    // Initialize game state (load all shaders/textures/levels)
    void Init();
    // GameLoop
    void ProcessInput(GLfloat dt);
    void Update(GLfloat dt);
    void Render();
    
};

#endif /* game_hpp */
