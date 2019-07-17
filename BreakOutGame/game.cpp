#include "game.hpp"


// Game-related State data
SpriteRenderer  *Renderer;
GameObject      *Player;
BallObject     *Ball;
ParticleGenerator     *Particles;
PostProcessor   *Effects;

GLfloat ShakeTime = 0.0f;

//ISoundEngine *SoundEngine = createIrrKlangDevice();

Game::Game(GLuint width, GLuint height)
: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
}

void Game::Init()
{

    // Load shaders
    ResourceManager::LoadShader("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/sprite.vs",
                                "/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/sprite.frag",
                                nullptr, "sprite");
    ResourceManager::LoadShader("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/particle.vs",
                                "/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/particle.frag", nullptr, "particle");
    ResourceManager::LoadShader("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/post_processing.vs",
                                "/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/shaders/post_processing.frag", nullptr, "postprocessing");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    // 加载纹理
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/background.jpg", GL_FALSE, "background");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/awesomeface.png", GL_TRUE, "face");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/block.png", GL_FALSE, "block");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/block_solid.png", GL_FALSE, "block_solid");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/paddle.png", GL_TRUE, "paddle");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/particle.png", GL_TRUE, "particle");

    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/powerup_speed.png", GL_TRUE, "powerup_speed");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/powerup_sticky.png", GL_TRUE, "powerup_sticky");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/powerup_increase.png", GL_TRUE, "powerup_increase");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/powerup_confuse.png", GL_TRUE, "powerup_confuse");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/powerup_chaos.png", GL_TRUE, "powerup_chaos");
    ResourceManager::LoadTexture("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/textures/powerup_passthrough.png", GL_TRUE, "powerup_passthrough");
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

    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
    
    // Configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
    //Effects->Shake = GL_TRUE;
    //Effects->Confuse = GL_TRUE;
    //Effects->Chaos = GL_TRUE;
    
//    SoundEngine->play2D("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/audio/breakout.mp3", GL_TRUE);
}

void Game::Update(GLfloat dt)
{
    Ball->Move(dt, this->Width);
    // 检测碰撞
    this->DoCollisions();

    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));
    this->UpdatePowerUps(dt);
    if (ShakeTime > 0.0f)
    {
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f)
            Effects->Shake = false;
    }
    if (Ball->Position.y >= this->Height) // 球是否接触底部边界？
    {
        this->ResetLevel();
        this->ResetPlayer();
    }
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

        Effects->BeginRender();
            // 绘制背景
            Renderer->DrawSprite(ResourceManager::GetTexture("background"),
                                 glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
                                 );
            // 绘制关卡
            this->Levels[this->Level].Draw(*Renderer);
            // Draw player
            // Draw PowerUps
            for (PowerUp &powerUp : this->PowerUps)
                if (!powerUp.Destroyed)
                    powerUp.Draw(*Renderer);
            Particles->Draw();
            Player->Draw(*Renderer);

            Ball->Draw(*Renderer);

        Effects->EndRender();
        Effects->Render(glfwGetTime());

    }
}

void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/one.lvl", this->Width, this->Height * 0.5f);
    else if (this->Level == 1)
        this->Levels[1].Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/two.lvl", this->Width, this->Height * 0.5f);
    else if (this->Level == 2)
        this->Levels[2].Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/three.lvl", this->Width, this->Height * 0.5f);
    else if (this->Level == 3)
        this->Levels[3].Load("/Users/daniel/CodeManager/BreakOutGame/BreakOutGame/levels/four.lvl", this->Width, this->Height * 0.5f);
}

void Game::ResetPlayer()
{
    // Reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2 - PLAYER_SIZE.x / 2, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -(BALL_RADIUS * 2)), INITIAL_BALL_VELOCITY);
    // Also disable all active powerups
    Effects->Chaos = Effects->Confuse = GL_FALSE;
    Ball->PassThrough = Ball->Sticky = GL_FALSE;
    Player->Color = glm::vec3(1.0f);
    Ball->Color = glm::vec3(1.0f);
}




// PowerUps
GLboolean isOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type);
void Game::UpdatePowerUps(GLfloat dt)
{
    for (PowerUp &powerUp : this->PowerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated)
        {
            powerUp.Duration -= dt;
            
            if (powerUp.Duration <= 0.0f)
            {
                // 之后会将这个道具移除
                powerUp.Activated = GL_FALSE;
                // 停用效果
                if (powerUp.Type == "sticky")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "sticky"))
                    {   // 仅当没有其他sticky效果处于激活状态时重置，以下同理
                        Ball->Sticky = GL_FALSE;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {
                        Ball->PassThrough = GL_FALSE;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "confuse"))
                    {
                        Effects->Confuse = GL_FALSE;
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "chaos"))
                    {
                        Effects->Chaos = GL_FALSE;
                    }
                }
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
                                        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
                                        ), this->PowerUps.end());
}


GLboolean ShouldSpawn(GLuint chance)
{
    GLuint random = rand() % chance;
    return random == 0;
}
void Game::SpawnPowerUps(GameObject &block)
{
    if (ShouldSpawn(75)) // 1/75的几率
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f),   0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
    if (ShouldSpawn(15)) // 负面道具被更频繁地生成
        this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
}


void ActivatePowerUp(PowerUp &powerUp)
{
    // 根据道具类型发动道具
    if (powerUp.Type == "speed")
    {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        Ball->Sticky = GL_TRUE;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        Ball->PassThrough = GL_TRUE;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        Player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse")
    {
        if (!Effects->Chaos)
            Effects->Confuse = GL_TRUE; // 只在chaos未激活时生效，chaos同理
    }
    else if (powerUp.Type == "chaos")
    {
        if (!Effects->Confuse)
            Effects->Chaos = GL_TRUE;
    }
}

GLboolean isOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
{
    // Check if another PowerUp of the same type is still active
    // in which case we don't disable its effect (yet)
    for (const PowerUp &powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return GL_TRUE;
    }
    return GL_FALSE;
}

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);
void ActivatePowerUp(PowerUp &powerUp);

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // 如果collision 是 true
            {
                // 如果不是实心的砖块则摧毁
                if (!box.IsSolid){
                    box.Destroyed = GL_TRUE;
                    this->SpawnPowerUps(box);
                }
                else
                {   // 如果是实心的砖块则激活shake特效
                    ShakeTime = 0.05f;
                    Effects->Shake = true;
                }
                // 碰撞处理
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT) // 水平方向碰撞
                {
                    Ball->Velocity.x = -Ball->Velocity.x; // 反转水平速度
                    // 重定位
                    GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        Ball->Position.x += penetration; // 将球右移
                    else
                        Ball->Position.x -= penetration; // 将球左移
                }
                else // 垂直方向碰撞
                {
                    Ball->Velocity.y = -Ball->Velocity.y; // 反转垂直速度
                    // 重定位
                    GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        Ball->Position.y -= penetration; // 将球上移
                    else
                        Ball->Position.y += penetration; // 将球下移
                }
            }
        }
    }
    
    for (PowerUp &powerUp : this->PowerUps)
    {
        if (!powerUp.Destroyed)
        {
            if (powerUp.Position.y >= this->Height)
                powerUp.Destroyed = GL_TRUE;
            if (CheckCollision(*Player, powerUp))
            {   // 道具与挡板接触，激活它！
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = GL_TRUE;
                powerUp.Activated = GL_TRUE;
            }
        }
    }
    
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        // 检查碰到了挡板的哪个位置，并根据碰到哪个位置来改变速度
        GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
        GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Player->Size.x / 2);
        // 依据结果移动
        GLfloat strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
        
        // If Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
        Ball->Stuck = Ball->Sticky;
    }
}
GLboolean CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // Collision x-axis?
    GLboolean collisionX = one.Position.x + one.Size.x >= two.Position.x &&
    two.Position.x + two.Size.x >= one.Position.x;
    // Collision y-axis?
    GLboolean collisionY = one.Position.y + one.Size.y >= two.Position.y &&
    two.Position.y + two.Size.y >= one.Position.y;
    // Collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // Get center point circle first
    glm::vec2 center(one.Position + one.Radius);
    // Calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // Get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // Now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // Now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;
    
    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
    else
        return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}

Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // 上
        glm::vec2(1.0f, 0.0f),  // 右
        glm::vec2(0.0f, -1.0f), // 下
        glm::vec2(-1.0f, 0.0f)  // 左
    };
    GLfloat max = 0.0f;
    GLuint best_match = -1;
    for (GLuint i = 0; i < 4; i++)
    {
        GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}
