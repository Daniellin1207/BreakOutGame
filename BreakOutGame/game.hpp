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
#include "game_level.hpp"
#include "ball_object_collisions.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};
// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100, 20);
// Initial velocity of the player paddle
const GLfloat PLAYER_VELOCITY(500.0f);

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
    
    
    std::vector<GameLevel> Levels;
    GLuint                 Level;
    
    
    // Constructor/Destructor
    Game(GLuint width, GLuint height);
    ~Game();
    // Initialize game state (load all shaders/textures/levels)
    void Init();
    // GameLoop
    void ProcessInput(GLfloat dt);
    void Update(GLfloat dt);
    void Render();
    
    void DoCollisions();
    
    
    
};

#endif /* game_hpp */
