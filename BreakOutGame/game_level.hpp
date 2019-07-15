//
//  game_level.hpp
//  BreakOutGame
//
//  Created by Daniel.Lin on 2019/7/15.
//  Copyright © 2019 Daniel.Lin. All rights reserved.
//

#ifndef game_level_hpp
#define game_level_hpp

#include <stdio.h>

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "game_object.hpp"
#include "sprite_renderer.hpp"
#include "resource_manager.hpp"


/// GameLevel holds all Tiles as part of a Breakout level and
/// hosts functionality to Load/render levels from the harddisk.
class GameLevel
{
public:
    // Level state
    std::vector<GameObject> Bricks;
    // Constructor
    GameLevel() { }
    // Loads level from file
    void      Load(const GLchar *file, GLuint levelWidth, GLuint levelHeight);
    // Render level
    void      Draw(SpriteRenderer &renderer);
    // Check if the level is completed (all non-solid tiles are destroyed)
    GLboolean IsCompleted();
private:
    // Initialize level from tile data
    void      init(std::vector<std::vector<GLuint>> tileData, GLuint levelWidth, GLuint levelHeight);
};

#endif /* game_level_hpp */
