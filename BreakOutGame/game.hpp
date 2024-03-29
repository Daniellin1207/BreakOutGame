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
#include "particle_generator.hpp"
#include "post_processor.hpp"
#include "power_up.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <iostream>
//#include <irrKlang.h>
//using namespace irrklang;

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};
// Defines a Collision typedef that represents collision data
typedef std::tuple<GLboolean, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100, 20);
// Initial velocity of the player paddle
const GLfloat PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const GLfloat BALL_RADIUS = 12.5f;

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
    
    std::vector<PowerUp>  PowerUps;
    
    void SpawnPowerUps(GameObject &block);
    void UpdatePowerUps(GLfloat dt);
    
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
    // Reset
    void ResetLevel();
    void ResetPlayer();
    
    
};

#endif /* game_hpp */
