//
//  game.cpp
//  BreakOutGame
//
//  Created by Daniel.Lin on 2019/7/14.
//  Copyright © 2019 Daniel.Lin. All rights reserved.
//

#include "game.hpp"


// Game-related State data
SpriteRenderer  *Renderer;
GameObject      *Player;
// 初始化球的速度
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// 球的半径
const GLfloat BALL_RADIUS = 12.5f;

BallObject     *Ball;

GLboolean CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // 获取圆的中心
    glm::vec2 center(one.Position + one.Radius);
    // 计算AABB的信息（中心、半边长）
    glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
    glm::vec2 aabb_center(
                          two.Position.x + aabb_half_extents.x,
                          two.Position.y + aabb_half_extents.y
                          );
    // 获取两个中心的差矢量
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // AABB_center加上clamped这样就得到了碰撞箱上距离圆最近的点closest
    glm::vec2 closest = aabb_center + clamped;
    // 获得圆心center和最近点closest的矢量并判断是否 length <= radius
    difference = closest - center;
    return glm::length(difference) < one.Radius;
}

Game::Game(GLuint width, GLuint height)
: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
    
}

Game::~Game()
{
    delete Renderer;
    delete Player;
}

void Game::Init()
{
    
    // Load shaders
    ResourceManager::LoadShader("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/sprite.vs",
                                "/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/sprite.frag",
                                nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // Load textures
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/awesomeface.png", GL_TRUE, "face");
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    
    // 加载纹理
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/background.jpg", GL_FALSE, "background");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/awesomeface.png", GL_TRUE, "face");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/block.png", GL_FALSE, "block");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/block_solid.png", GL_FALSE, "block_solid");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/paddle.png", GL_TRUE, "paddle");
    // 加载关卡
    GameLevel one; one.Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/one.lvl", this->Width, this->Height * 0.5);
    GameLevel two; two.Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/two.lvl", this->Width, this->Height * 0.5);
    GameLevel three; three.Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/three.lvl", this->Width, this->Height * 0.5);
    GameLevel four; four.Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/four.lvl", this->Width, this->Height * 0.5);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;
    
    // Configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
                          ResourceManager::GetTexture("face"));
}

void Game::Update(GLfloat dt)
{
    Ball->Move(dt, this->Width);
    // 检测碰撞
    this->DoCollisions();
}


void Game::ProcessInput(GLfloat dt)
{
    if (this->State == GAME_ACTIVE)
    {
        GLfloat velocity = PLAYER_VELOCITY * dt;
        // 移动玩家挡板
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0)
            {
                Player->Position.x -= velocity;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= this->Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }
}

void Game::Render()
{
//    Renderer->DrawSprite(ResourceManager::GetTexture("face"), glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    
    if(this->State == GAME_ACTIVE)
    {
        // 绘制背景
        Renderer->DrawSprite(ResourceManager::GetTexture("background"),
                             glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
                             );
        // 绘制关卡
        this->Levels[this->Level].Draw(*Renderer);
        // Draw player
        Player->Draw(*Renderer);
        
        Ball->Draw(*Renderer);
    }
}

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            if (CheckCollision(*Ball, box))
            {
                if (!box.IsSolid)
                    box.Destroyed = GL_TRUE;
            }
        }
    }
}
