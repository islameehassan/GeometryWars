#include "Game.h"


Game::Game(const std::string& config, const std::string& highscore){
    srand(time(nullptr));
    m_highScoreFilePath = highscore;
    init(config);
}

void Game::init(const std::string& config)
{   
    std::ifstream in(config);

    if(!in.is_open())
    {
        std::cerr << "could not open the config file\n";
        exit(-1);
    }

    std::string mode;

    std::string fontFileName;
    int fontSize;
    int r, g, b;

    while(in >> mode)
    {
        if(mode == "Window")
        {
            int frameRate;
            bool fullScreen;

            in >> m_windowWidth >> m_windowHeight >> frameRate >> fullScreen;
            m_window.create(sf::VideoMode(m_windowWidth, m_windowHeight), "Geomtery Wars",
                         (fullScreen ? sf::Style::Fullscreen: sf::Style::Default));
            if(fullScreen)
            {
                m_windowWidth = 1920;   // replace by your monitor size
                m_windowHeight = 1080;
            }
            m_window.setFramerateLimit(frameRate);
            ImGui::SFML::Init(m_window);
        }
        else if(mode == "Player")
        {
            in >> m_playerConfig.SRadius >> m_playerConfig.CRadius >> m_playerConfig.Speed
               >> m_playerConfig.FRed >> m_playerConfig.FGreen >> m_playerConfig.FBlue 
               >> m_playerConfig.ORed >> m_playerConfig.OGreen >> m_playerConfig.OBlue
               >> m_playerConfig.OThickness >> m_playerConfig.Vertices;
        }
        else if(mode == "Enemy")
        {
            in >> m_enemyConfig.SRadius >> m_enemyConfig.CRadius >> m_enemyConfig.SMIN
               >> m_enemyConfig.SMAX >> m_enemyConfig.ORed >> m_enemyConfig.OGreen 
               >> m_enemyConfig.OBlue >> m_enemyConfig.OThickness >> m_enemyConfig.VMIN 
               >> m_enemyConfig.VMAX >> m_enemyConfig.Lifespan >> m_enemyConfig.SIntrval;
        }
        else if(mode == "Bullet")
        {
            in >> m_bulletConfig.SRadius >> m_bulletConfig.CRadius >> m_bulletConfig.Speed 
               >> m_bulletConfig.FRed >> m_bulletConfig.FGreen >> m_bulletConfig.FBlue
               >> m_bulletConfig.ORed >> m_bulletConfig.OGreen >> m_bulletConfig.OBlue
               >> m_bulletConfig.OThickness >> m_bulletConfig.Vertices >> m_bulletConfig.Lifespan
               >> m_bulletConfig.ShRate;
        }
        else if(mode == "Font")
        {
            in >> fontFileName >> fontSize >> r >> g >> b;
        }
        else
        {
            std::cerr << "undefined format\n";
            exit(-1);
        }
    }

    std::ifstream hin(m_highScoreFilePath);
    if(!hin.is_open())
    {
        std::cerr << "could not open the high score file\n";
        exit(-1);
    }

    mode = "";
    while(hin >> mode)
    {
        if(mode == "highscore")
        {
            hin >> m_highScore;
        }
        else
        {
            std::cerr << "undefined mode\n";
            exit(-1);
        }
    }

    m_lastBulletShootFrame = -20;
    m_currentFrame = 0;
    m_lastEnemySpawningFrame = -300;
    m_score = 0;
    m_running = true;
    m_pause = false;
    m_isMovement = true;
    m_isLifespan = true;
    m_isCollision = true;
    m_isSpawning = true;
    m_isRendering = true;
    m_isGUI = true;
    m_GUIDisabledThisFrame = false;
    m_blackHole = nullptr;

    if(!m_font.loadFromFile(fontFileName))
    {
        std::cerr << "could not load the font file\n";
        exit(-1);
    }

    m_scoreText.setFont(m_font);
    m_scoreText.setFillColor(sf::Color(r, g, b));
    m_scoreText.setCharacterSize(fontSize);
    m_scoreText.setPosition(0, 5);
    m_scoreText.setString("Score: " + std::to_string(m_score));

    m_highScoreText.setFont(m_font);
    m_highScoreText.setFillColor(sf::Color(r, g, b));
    m_highScoreText.setCharacterSize(fontSize);
    m_highScoreText.setString("High Score" + std::to_string(m_highScore));
    m_highScoreText.setPosition(m_windowWidth - m_highScoreText.getLocalBounds().width - m_highScoreText.getLocalBounds().left, 5);

    spawnPlayer();
}


void Game::spawnPlayer()
{
    
    auto e = m_entityManager.add("player");

    e->Shape = std::make_shared<cShape>(m_playerConfig.SRadius, m_playerConfig.Vertices, sf::Color(m_playerConfig.FRed, m_playerConfig.FGreen, m_playerConfig.FBlue),
                                        sf::Color(m_playerConfig.ORed, m_playerConfig.OGreen, m_playerConfig.OBlue), m_playerConfig.OThickness);

    e->Transform = std::make_shared<cTransform>(Vec2(m_windowWidth/2.0f, m_windowHeight/2.0f), Vec2(0, 0), 0);

    e->Collision = std::make_shared<cCollision>(m_playerConfig.CRadius);

    e->Input = std::make_shared<cInput>();
    

    e->SpecialAbilities.push_back(std::make_shared<cSpecialAbility>(SpecialAbilityType::INSTANT_DAMAGE, 1000.f, 0.f));
    e->SpecialAbilities.push_back(std::make_shared<cSpecialAbility>(SpecialAbilityType::STATIC_BLACK_HOLE, 500.f, 0.f));
    e->SpecialAbilities.push_back(std::make_shared<cSpecialAbility>(SpecialAbilityType::MOVING_BLACK_HOLE, 250.f, 0.f));
    e->SpecialAbilities.push_back(std::make_shared<cSpecialAbility>(SpecialAbilityType::AI_COMPANION, 100000.f, 0.f));

    m_player = e;
}

void Game::sEnemySpawner()
{
    if(!m_isSpawning)
    {
        return;
    }

    if(m_currentFrame - m_lastEnemySpawningFrame < m_enemyConfig.SIntrval && !m_isManualSpawning)
    {
        return;
    }
    
    auto e = m_entityManager.add("enemy");

    int segments;
    u_int8_t fill_r, fill_g, fill_b;
    int angle, speed, x, y;
    Vec2 velocity;

    fill_r      = rand() % 255;
    fill_g      = rand() % 255;
    fill_b      = rand() % 255;
    angle       = rand() % 360;
    speed       = m_enemyConfig.SMIN    + rand() % (int)(m_enemyConfig.SMAX - m_enemyConfig.SMIN + 1);
    segments    = m_enemyConfig.VMIN    + rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN);
    x           = m_enemyConfig.SRadius + rand() % (m_windowWidth - 2 * m_enemyConfig.SRadius);
    y           = m_enemyConfig.SRadius + rand() % (m_windowHeight - 2 * m_enemyConfig.SRadius);
    
    velocity.x = speed * cos((angle/180.f) * PI);
    velocity.y = speed * sin((angle/180.f) * PI);

    e->Shape = std::make_shared<cShape>(m_enemyConfig.SRadius, segments, sf::Color(fill_r, fill_g, fill_b), 
                                        sf::Color(m_enemyConfig.ORed, m_enemyConfig.OGreen, m_enemyConfig.OBlue), m_enemyConfig.OThickness);
    
    e->Transform = std::make_shared<cTransform>(Vec2(x, y), velocity, angle);

    e->Collision = std::make_shared<cCollision>(m_enemyConfig.CRadius);

    e->Score = std::make_shared<cScore>(segments * 100);

    m_lastEnemySpawningFrame = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> src)
{
    int numSmallEnemies = src->Shape->circle.getPointCount();

    float speed = src->Transform->velocity.length();

    int angle = 0;

    int increment = 360/numSmallEnemies;

    for(int i = 1; i <= numSmallEnemies; ++i)
    {
        auto e = m_entityManager.add("small_enemy");

        e->Shape = std::make_shared<cShape>(src->Shape->circle.getRadius()/6, numSmallEnemies, 
                                            src->Shape->circle.getFillColor(), src->Shape->circle.getOutlineColor()
                                            , src->Shape->circle.getOutlineThickness());
        
        e->Collision = std::make_shared<cCollision>(src->Collision->radius/4);

        e->Score = std::make_shared<cScore>(src->Score->score * 2);

        e->Lifespan = std::make_shared<cLifespan>(m_enemyConfig.Lifespan, m_enemyConfig.Lifespan);

        Vec2 velocity = Vec2(speed * cos((angle/180.f) * PI), speed * sin((angle/180.f) * PI));

        e->Transform = std::make_shared<cTransform>(src->Transform->pos, velocity, angle);

        angle += increment;
    }
}

void Game::spawnBullet(Vec2 dest)
{
    if(m_currentFrame - m_lastBulletShootFrame < m_bulletConfig.ShRate)
    {
        return;
    }

    Vec2 velocity = dest - m_player->Transform->pos;
    velocity.normalize();
    velocity *= m_bulletConfig.Speed;    // bullet's speed
    int angle = tanf(velocity.y/velocity.x);


    auto e = m_entityManager.add("bullet");

    e->Shape = std::make_shared<cShape>(m_bulletConfig.SRadius, m_bulletConfig.Vertices, sf::Color(m_bulletConfig.FRed, m_bulletConfig.FGreen, m_bulletConfig.FBlue),
                                         sf::Color(m_bulletConfig.ORed, m_bulletConfig.OGreen, m_bulletConfig.OBlue), m_bulletConfig.OThickness);

    e->Transform = std::make_shared<cTransform>(Vec2(m_player->Transform->pos.x, m_player->Transform->pos.y),
                                                velocity, angle);

    e->Collision = std::make_shared<cCollision>(m_bulletConfig.CRadius);

    e->Lifespan = std::make_shared<cLifespan>(m_bulletConfig.Lifespan, m_bulletConfig.Lifespan);

    m_lastBulletShootFrame = m_currentFrame;
}

void Game::spawnBlackHole(bool isMoving)
{
    std::string tag = (isMoving ? "wandering_blackhole": "static_blackhole");
    auto e = m_entityManager.add(tag);

    int speed, angle, x, y, radius;

    speed = 1 + rand() % 3;
    angle = rand() % 360;
    radius = 100 + rand() % 31;
    x  = radius + rand() % (m_windowWidth - 2 * radius);
    y  = radius + rand() % (m_windowHeight - 2 * radius);
    Vec2 velocity = {speed * cos((angle)/180.0 * PI), speed * sin((angle)/180.0 * PI)};

    velocity = (isMoving ? velocity: Vec2(0,0));
    angle = (isMoving ? angle: 0);

    e->Transform = std::make_shared<cTransform>(Vec2(x,y), velocity, angle);

    e->Shape = std::make_shared<cShape>(radius, 60, sf::Color::Black, sf::Color(119,0,0), 10.0);

    e->Collision = std::make_shared<cCollision>(radius);

    e->Lifespan = std::make_shared<cLifespan>(2000,2000);

    float rangeRadius = (isMoving ? radius + 400: 10000);    // static holes are more powerful
    e->Gravity = std::make_shared<cGravity>(5, rangeRadius);

    m_blackHole = e;
}

void Game::sUserInput()
{
    sf::Event event;

    while(m_window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);

        if(event.type == sf::Event::KeyPressed)
        {
            switch(event.key.code)
            {
                case sf::Keyboard::W:
                {
                    m_player->Input->up = true;
                    break;
                }
                case sf::Keyboard::S:
                {
                    m_player->Input->down = true;
                    break;
                }
                case sf::Keyboard::D:
                {
                    m_player->Input->right = true;
                    break;
                }
                case sf::Keyboard::A:
                {
                    m_player->Input->left = true;
                    break;
                }
                case sf::Keyboard::Escape:
                {
                    m_running = false;
                    break;
                }
                case sf::Keyboard::P:       // pause should disables movement and spawning
                {
                    if(m_isMovement || m_isSpawning)
                    {
                        m_isMovement = m_isSpawning = false;
                    }
                    else
                    {
                        m_isMovement = m_isSpawning = true;
                    }
                    break;
                }
                case sf::Keyboard::G:
                {
                    m_isGUI = !m_isGUI;
                    break;
                }
                default:
                    break;
            }
        }
        else if(event.type == sf::Event::KeyReleased)
        {
            switch(event.key.code)
            {
                case sf::Keyboard::W:
                {
                    m_player->Input->up = false;
                    break;
                }
                case sf::Keyboard::S:
                {
                    m_player->Input->down = false;
                    break;
                }
                case sf::Keyboard::D:
                {
                    m_player->Input->right = false;
                    break;
                }
                case sf::Keyboard::A:
                {
                    m_player->Input->left = false;
                    break;
                }
                default:
                    break;
            }   
        }
        else if(event.type == sf::Event::MouseButtonPressed)
        {
            if(event.mouseButton.button == sf::Mouse::Left && !ImGui::GetIO().WantCaptureMouse)
            {
                m_player->Input->shooting = true;
            }
            else if(event.mouseButton.button == sf::Mouse::Right && !ImGui::GetIO().WantCaptureMouse)
            {
                m_player->Input->specialAbility = true;
            }
        }
        else if(event.type == sf::Event::MouseButtonReleased)
        {
            if(event.mouseButton.button == sf::Mouse::Left)
            {
                m_player->Input->shooting = false;
            }
            else if(event.mouseButton.button == sf::Mouse::Right && !ImGui::GetIO().WantCaptureMouse)
            {
                m_player->Input->specialAbility = false;
            }
        }
        else if(event.type == sf::Event::Closed)
        {
            m_running = false;
        }
    }
}

void Game::sRender()
{
    if(!m_isRendering)
    {
        ImGui::SFML::Render(m_window);
        return;
    }

    EntityVec entitites = m_entityManager.getEntities();

    for(auto& e: entitites)
    {
        if(e->isActive())
        {
            e->Shape->circle.setPosition(e->Transform->pos.x, e->Transform->pos.y);
            e->Shape->circle.setRotation(e->Transform->rotation);
        }      
    }
    m_scoreText.setString("Score: " + std::to_string(m_score));
    m_highScoreText.setString("High Score: " + std::to_string(m_highScore));
    m_highScoreText.setPosition(m_windowWidth - m_highScoreText.getLocalBounds().width - m_highScoreText.getLocalBounds().left, 5);

    

    m_window.clear(sf::Color(30, 10, 10));
    for(auto& e: entitites)
    {
        if(e->isActive())
        {
            m_window.draw(e->Shape->circle);
        }
    }
    m_window.draw(m_scoreText);
    m_window.draw(m_highScoreText);

    if(m_isGUI || m_GUIDisabledThisFrame)
    {
        ImGui::SFML::Render(m_window);
        m_GUIDisabledThisFrame = false;
    }

    m_window.display();

}

void Game::sGUI()
{
    if(!m_isGUI)
    {
        return;
    }

    EntityVec allEntities = m_entityManager.getEntities();
    EntityVec allEnemies = m_entityManager.getEntitiesByTag("enemy");
    EntityVec allBullets = m_entityManager.getEntitiesByTag("bullet");
    EntityVec allSmallEnemies = m_entityManager.getEntitiesByTag("small_enemy");
    EntityVec blackHoles = m_entityManager.getEntitiesByTag("static_blackhole");
    blackHoles.insert(blackHoles.end(),m_entityManager.getEntitiesByTag("wandering_blackhole").begin()
                        , m_entityManager.getEntitiesByTag("wandering_blackhole").end());

    ImGui::SFML::Update(m_window, m_deltaClock.restart());

    ImGui::Begin("Configuration");
    ImGui::BeginTabBar("1");


    if(ImGui::BeginTabItem("Systems"))
    {
        ImGui::Checkbox("Movement", &m_isMovement);
        ImGui::Checkbox("Lifespan", &m_isLifespan);
        ImGui::Checkbox("Collision", &m_isCollision);
        ImGui::Checkbox("Spawning", &m_isSpawning);
        ImGui::SliderInt("Spawn Interval", &m_enemyConfig.SIntrval, 20, 300);
        m_isManualSpawning = ImGui::Button("Manual Spawn");
        m_GUIDisabledThisFrame = ImGui::Checkbox("GUI", &m_isGUI);
        ImGui::Checkbox("Rendering", &m_isRendering);
        ImGui::EndTabItem();
    }
    
    if(ImGui::BeginTabItem("Entities"))
    {
        if(ImGui::CollapsingHeader("Entities by Tag", ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            ImGui::Indent();
            if(ImGui::CollapsingHeader("Bullet"))
            {
                ImGui::Indent();
                for(size_t i = 0; i < allBullets.size(); ++i)
                {
                    sf::Color c = allBullets[i]->Shape->circle.getFillColor();
                    ImGui::PushStyleColor(ImGuiCol_Button,  ImVec4(c.r/255.0, c.g/255.0, c.b/255.0, 1.0));
                    std::string id = "D##" + std::to_string((int) allBullets[i]->id());
                    bool destroy = ImGui::Button(id.c_str(), ImVec2(20.0f, 20.0f));
                    ImGui::SameLine(0, 10.0f);
                    ImGui::Text(std::to_string(allBullets[i]->id()).c_str());
                    ImGui::SameLine(0, 20.f);
                    int x = allBullets[i]->Shape->circle.getPosition().x;
                    int y = allBullets[i]->Shape->circle.getPosition().y;
                    ImGui::Text(("(" + std::to_string(x) + "," + std::to_string(y) + ")").c_str());
                    
                    ImGui::PopStyleColor(1);
                    if(destroy)
                    {
                        allBullets[i]->destroy();
                    }
                }
                ImGui::Unindent();
            }


            if(ImGui::CollapsingHeader("Enemy"))
            {
                ImGui::Indent();
                for(size_t i = 0; i < allEnemies.size(); ++i)
                {
                    sf::Color c = allEnemies[i]->Shape->circle.getFillColor();
                    ImGui::PushStyleColor(ImGuiCol_Button,  ImVec4(c.r/255.0, c.g/255.0, c.b/255.0, 1.0));
                    std::string id = "D##" + std::to_string((int) allEnemies[i]->id());
                    bool destroy = ImGui::Button(id.c_str(), ImVec2(20.0f, 20.0f));
                    ImGui::SameLine(0, 10.0f);
                    ImGui::Text(std::to_string(allEnemies[i]->id()).c_str());
                    ImGui::SameLine(0, 20.f);
                    int x = allEnemies[i]->Shape->circle.getPosition().x;
                    int y = allEnemies[i]->Shape->circle.getPosition().y;
                    ImGui::Text(("(" + std::to_string(x) + "," + std::to_string(y) + ")").c_str());
                    
                    ImGui::PopStyleColor(1);
                    if(destroy)
                    {
                        allEnemies[i]->destroy();
                    }
                }
                ImGui::Unindent();
            }


            if(ImGui::CollapsingHeader("Player"))
            {
                ImGui::Indent();
                sf::Color c = m_player->Shape->circle.getFillColor();
                ImGui::PushStyleColor(ImGuiCol_Button,  ImVec4(c.r/255.0, c.g/255.0, c.b/255.0, 1.0));
                std::string id = "D##" + std::to_string((int) m_player->id());
                bool destroy = ImGui::Button(id.c_str(), ImVec2(20.0f, 20.0f));
                ImGui::SameLine(0, 10.0f);
                ImGui::Text(std::to_string(m_player->id()).c_str());
                ImGui::SameLine(0, 20.f);
                int x = m_player->Shape->circle.getPosition().x;
                int y = m_player->Shape->circle.getPosition().y;
                ImGui::Text(("(" + std::to_string(x) + "," + std::to_string(y) + ")").c_str());
                
                ImGui::PopStyleColor(1);
                if(destroy)
                {
                    m_player->destroy();
                }
                ImGui::Unindent();
            }


            if(ImGui::CollapsingHeader("Small Enemy"))
            {
                ImGui::Indent();
                for(size_t i = 0; i < allSmallEnemies.size(); ++i)
                {
                    sf::Color c = allSmallEnemies[i]->Shape->circle.getFillColor();
                    ImGui::PushStyleColor(ImGuiCol_Button,  ImVec4(c.r/255.0, c.g/255.0, c.b/255.0, 1.0));
                    std::string id = "D##" + std::to_string((int) allSmallEnemies[i]->id());
                    bool destroy = ImGui::Button(id.c_str(), ImVec2(20.0f, 20.0f));
                    ImGui::SameLine(0, 10.0f);
                    ImGui::Text(std::to_string(allSmallEnemies[i]->id()).c_str());
                    ImGui::SameLine(0, 20.f);
                    int x = allSmallEnemies[i]->Shape->circle.getPosition().x;
                    int y = allSmallEnemies[i]->Shape->circle.getPosition().y;
                    ImGui::Text(("(" + std::to_string(x) + "," + std::to_string(y) + ")").c_str());
                    
                    ImGui::PopStyleColor(1);
                    if(destroy)
                    {
                        allSmallEnemies[i]->destroy();
                    }
                }
                ImGui::Unindent();
            }


            if(ImGui::CollapsingHeader("Blackholes"))
            {
                ImGui::Indent();
                for(size_t i = 0; i < blackHoles.size(); ++i)
                {
                    sf::Color c = blackHoles[i]->Shape->circle.getFillColor();
                    ImGui::PushStyleColor(ImGuiCol_Button,  ImVec4(c.r/255.0, c.g/255.0, c.b/255.0, 1.0));
                    std::string id = "D##" + std::to_string((int) blackHoles[i]->id());
                    bool destroy = ImGui::Button(id.c_str(), ImVec2(20.0f, 20.0f));
                    ImGui::SameLine(0, 10.0f);
                    ImGui::Text(std::to_string(blackHoles[i]->id()).c_str());
                    ImGui::SameLine(0, 20.f);
                    int x = blackHoles[i]->Shape->circle.getPosition().x;
                    int y = blackHoles[i]->Shape->circle.getPosition().y;
                    ImGui::Text(("(" + std::to_string(x) + "," + std::to_string(y) + ")").c_str());
                    
                    ImGui::PopStyleColor(1);
                    if(destroy)
                    {
                        blackHoles[i]->destroy();
                    }
                }
                ImGui::Unindent();
            }

            ImGui::Unindent();
        }
        if(ImGui::CollapsingHeader("Entities"))
        {
            ImGui::Indent();
            for(size_t i = 0; i < allEntities.size(); ++i)
            {
                sf::Color c = allEntities[i]->Shape->circle.getFillColor();
                ImGui::PushStyleColor(ImGuiCol_Button,  ImVec4(c.r/255.0, c.g/255.0, c.b/255.0, 1.0));
                std::string id = "D##" + std::to_string((int) allEntities[i]->id());
                bool destroy = ImGui::Button(id.c_str(), ImVec2(20.0f, 20.0f));
                ImGui::SameLine(0, 10.0f);
                ImGui::Text(std::to_string(allEntities[i]->id()).c_str());
                ImGui::SameLine(0, 20.f);
                ImGui::Text(allEntities[i]->tag().c_str());
                ImGui::SameLine(0, 20.0f);
                int x = allEntities[i]->Shape->circle.getPosition().x;
                int y = allEntities[i]->Shape->circle.getPosition().y;
                ImGui::Text(("(" + std::to_string(x) + "," + std::to_string(y) + ")").c_str());
                
                ImGui::PopStyleColor(1);
                if(destroy)
                {
                    allEntities[i]->destroy();
                }
            }
            ImGui::Unindent();
        }
        ImGui::EndTabItem();
    }

    if(ImGui::BeginTabItem("Special Abilities"))
    {

        ImGui::RadioButton("Instant Damage", &m_player->Input->specialAbilityIndex,
                             0);
        ImGui::SameLine(0, 20.0);
        ImGui::ProgressBar(m_player->SpecialAbilities[0]->elapsed/(m_player->SpecialAbilities[0]->cooldownPeriod * 1.0f));


        ImGui::RadioButton("Static Blackhole", &m_player->Input->specialAbilityIndex, 
                            1);
        ImGui::SameLine(0, 20.0);
        ImGui::ProgressBar(m_player->SpecialAbilities[1]->elapsed/(m_player->SpecialAbilities[1]->cooldownPeriod * 1.0f));


        ImGui::RadioButton("Wandering Blackhole", &m_player->Input->specialAbilityIndex,
                           2);
        ImGui::SameLine(0, 20.0);
        ImGui::ProgressBar(m_player->SpecialAbilities[2]->elapsed/(m_player->SpecialAbilities[2]->cooldownPeriod * 1.0f));


        ImGui::RadioButton("AI Guard", &m_player->Input->specialAbilityIndex,
                             3);
        ImGui::SameLine(0, 20.0);
        ImGui::ProgressBar(m_player->SpecialAbilities[3]->elapsed/(m_player->SpecialAbilities[3]->cooldownPeriod * 1.0f));


        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();


}

void Game::sLifespan()
{
    if(!m_isLifespan)
    {
        return;
    }

    for(auto& e: m_entityManager.getEntities())
    {
        if(e->Lifespan && e->isActive())
        {
            if(e->tag() != "enemy" || e->tag() != "player")
            {
                if(e->Lifespan->remaining == 0)
                {
                    e->destroy();
                    continue;
                }

                int total = e->Lifespan->total;
                int remaining = e->Lifespan->remaining;

                sf::Color c = e->Shape->circle.getFillColor();
                sf::Color o = e->Shape->circle.getOutlineColor();
                e->Shape->circle.setFillColor(sf::Color(c.r, c.g, c.b, remaining/(total * 1.0) * 255));
                e->Shape->circle.setOutlineColor(sf::Color(o.r, o.g, o.b, remaining/(total * 1.0) * 255));

                e->Lifespan->remaining--;
            }
        }
    }
}

void Game::sMovement()
{
    if(!m_isMovement)
    {
        return; // system is disabled
    }

    for(auto& e: m_entityManager.getEntities())
    {
        if(e->tag() == "enemy" || e->tag() == "bullet" || e->tag() == "small_enemy" || e->tag() == "wandering_blackhole")
        {
            e->Transform->rotation += 2;
            e->Transform->pos += e->Transform->velocity;
        }
        else if(e->tag() == "player")
        {
            e->Transform->rotation += 2;

            Vec2 velocity = m_player->Transform->velocity;

            if(m_player->Input->up && velocity.y != -m_playerConfig.Speed)
            {
                velocity += Vec2(0, -m_playerConfig.Speed); 
            }
            if(m_player->Input->down && velocity.y != m_playerConfig.Speed)
            {
                velocity += Vec2(0, m_playerConfig.Speed);
            }
            if(m_player->Input->right && velocity.x != m_playerConfig.Speed)
            {
                velocity += Vec2(m_playerConfig.Speed, 0);
            }
            if(m_player->Input->left && velocity.x != -m_playerConfig.Speed)
            {
                velocity += Vec2(-m_playerConfig.Speed, 0);
            }

            if(velocity == m_player->Transform->velocity)   // no button pressed
            {
                velocity = Vec2(0, 0);
            }

            m_player->Transform->velocity = velocity;
            m_player->Transform->pos += velocity;
        }
    }
}

void Game::sPlayer()
{
    if(!m_player->isActive())
    {
        spawnPlayer();
        return;
    }

    if(m_player->Input->shooting)
    {
        // get the current mouse position in the window
        sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);

        // convert it to world coordinates
        sf::Vector2f worldPos = m_window.mapPixelToCoords(pixelPos);

        Vec2 mousePosition = {worldPos.x, worldPos.y};
        spawnBullet(mousePosition);
    }
}

bool Game::isCollision(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2)
{
    if(!e1->Collision || !e2->Collision)
    {
        return false;
    }

    Vec2 center1 = Vec2(e1->Shape->circle.getPosition().x, e1->Shape->circle.getPosition().y);
    Vec2 center2 = Vec2(e2->Shape->circle.getPosition().x, e2->Shape->circle.getPosition().y);

    float collisionRadius1 = e1->Collision->radius;
    float collisionRadius2 = e2->Collision->radius;

    Vec2 distanceBetweenCenters = Vec2(center1 - center2);

    float distanceSquared = distanceBetweenCenters.x * distanceBetweenCenters.x +
                            distanceBetweenCenters.y * distanceBetweenCenters.y;

    return (distanceSquared < (collisionRadius1 + collisionRadius2) * (collisionRadius1 + collisionRadius2));
}

void Game::sCollision()
{
    if(!m_isCollision)
    {
        return;
    }

    EntityVec entities = m_entityManager.getEntities();


    for(auto& e: entities)
    {
        if(e->tag() != "bullet" && e->tag() != "small_enemy" && e->Collision){
            if(e->Transform->pos.y - e->Shape->circle.getRadius() < 0)
            {
                e->Transform->pos.y = e->Shape->circle.getRadius();
                e->Transform->velocity.y *= -1;
            }

            if(e->Transform->pos.x - e->Shape->circle.getRadius() < 0)
            {
                e->Transform->pos.x = e->Shape->circle.getRadius();
                e->Transform->velocity.x *= -1;
            }
            if(e->Transform->pos.y + e->Shape->circle.getRadius() > m_windowHeight)
            {
                e->Transform->pos.y = m_windowHeight - e->Shape->circle.getRadius();
                e->Transform->velocity.y *= -1;
            }

            if(e->Transform->pos.x + e->Shape->circle.getRadius() > m_windowWidth)
            {
                e->Transform->pos.x = m_windowWidth - e->Shape->circle.getRadius();
                e->Transform->velocity.x *= -1;
            }
        }
    }

    for(auto& e1: entities)
    {
        for(auto e2: entities)
        {
            if(e1->isActive() && e2->isActive() && e1->Collision && e2->Collision)
            {
                if(e1->tag() == "enemy" || e1->tag() == "small_enemy")
                {
                    if((e2->tag() == "bullet" || e2->tag() == "player") && isCollision(e1,e2))
                    {
                        e1->destroy();
                        e2->destroy();
                    }
                    else if((e2->tag() == "static_blackhole" || e2->tag() == "wandering_blackhole") && isCollision(e1,e2))
                    {
                        e1->destroy();
                    }
                }
            }
        }
    }

    // in general, spawn small enemies if an enemy is destroyed
    for(auto e: entities)
    {
        int index = 0;
        if(e->tag() == "enemy" && e->isActive() == false)
        {
            spawnSmallEnemies(e);
        }
    }

}

void Game::sScore()
{
    if(!m_player->isActive())
    {
        return;
    }

    for(auto& e: m_entityManager.getEntities())
    {
        if((e->tag() == "enemy" || e->tag() == "small_enemy") && e->isActive() == false)
        {
            m_score += e->Score->score;
        }
    }

    if(m_score < 0)
        m_score = 0;
    
    if(m_score > m_highScore)
    {
        m_highScore = m_score;
    }
}

void Game::sSpecialAbility()
{
    for(auto e: m_player->SpecialAbilities)
    {
        if(e->elapsed < e->cooldownPeriod)
        {        
            e->elapsed++;
        }
    }

    if(m_player->Input->specialAbility == false)
    {
        return;
    }

    bool abilityActivitated = false;
    if(m_player->Input->specialAbilityIndex == 0 && m_player->SpecialAbilities[0]->elapsed == m_player->SpecialAbilities[0]->cooldownPeriod)     // Instant Damage
    {
        for(auto e: m_entityManager.getEntities())
        {
            if(e->tag() != "player")
            {
                e->destroy();
            }
        }
        abilityActivitated = true;

    }
    else if(m_player->Input->specialAbilityIndex == 1 && m_player->SpecialAbilities[1]->elapsed == m_player->SpecialAbilities[1]->cooldownPeriod)  // Static Black Hole
    {
        spawnBlackHole(false);
        abilityActivitated = true;
    }
    else if(m_player->Input->specialAbilityIndex == 2 && m_player->SpecialAbilities[2]->elapsed == m_player->SpecialAbilities[2]->cooldownPeriod)  // Moving Black Hole
    {
        spawnBlackHole(true);
        abilityActivitated = true;
    }
    // else if(m_player->Input->specialAbilityIndex == 3)  // AI_Companion
    // {
    //     // TODO: spawn a companion around you which damages any enemies 
    //     // in diameter x of you.
    // }
    
    if(abilityActivitated)
    {
        for(auto e: m_player->SpecialAbilities)
        {
            e->elapsed = 0;
        }
    }
}

void Game::sGravity()
{
    if(!m_blackHole || !m_blackHole->isActive())    // black hole not created yet
    {
        return;
    }


    for(auto e: m_entityManager.getEntities())
    {

        if((e->tag() == "enemy" || e->tag() == "small_enemy") && e->isActive())
        {
            Vec2 enemyPos = e->Transform->pos;
            Vec2 blackHolePos = m_blackHole->Transform->pos;

            Vec2 posVector = blackHolePos - enemyPos;
            if(e->tag() == "static_blackhole" || posVector.length() < m_blackHole->Gravity->rangeRadius)
            {
                float speed = m_blackHole->Gravity->gravity;    // sorry physics 
                speed += e->Transform->velocity.length();

                posVector.normalize();

                posVector *= speed;

                e->Transform->velocity = posVector;
            }
        }
    }

}

void Game::run()
{
    while(m_running)
    {
        m_entityManager.update();

        sUserInput();
        sGUI();
        sPlayer();
        sSpecialAbility();
        sGravity();
        sEnemySpawner();
        sMovement();
        sCollision();
        sScore();
        sLifespan();
        sRender();

        m_currentFrame++;
    }
}


Game::~Game()
{
    if(m_score == m_highScore)
    {
        std::ofstream hout(m_highScoreFilePath);
        hout << "highscore" << " " << m_score;
    }
}