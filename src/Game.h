#include <SFML/Graphics.hpp>
#include <fstream>
#include <time.h>
#include "EntityManager.h"
#include "Components.hpp"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui.h"

struct PlayerConfig {int SRadius, CRadius, FRed, FGreen, FBlue, ORed, OGreen, OBlue, OThickness, Vertices;
                    float Speed;};

struct EnemyConfig {int SRadius, CRadius, ORed, OGreen, OBlue, OThickness, VMIN, VMAX, Lifespan, SIntrval; float SMIN, SMAX;};

struct BulletConfig {int SRadius, CRadius, FRed, FGreen, FBlue, ORed, OGreen, OBlue, OThickness, Vertices, Lifespan, ShRate; float Speed;};

#define PI 3.141

class Game
{
    sf::RenderWindow m_window;
    sf::Text m_scoreText;
    sf::Text m_highScoreText;
    sf::Font m_font;
    sf::Clock m_deltaClock;
    EntityManager m_entityManager;
    std::shared_ptr<Entity> m_player;   
    std::shared_ptr<Entity> m_blackHole;
    PlayerConfig m_playerConfig;
    EnemyConfig m_enemyConfig;
    BulletConfig m_bulletConfig;
    std::string m_highScoreFilePath;
    int m_windowWidth;
    int m_windowHeight;
    int m_currentFrame;
    int m_lastEnemySpawningFrame;
    int m_lastBulletShootFrame;
    int m_score;
    int m_highScore;
    bool m_running;
    bool m_pause;
    bool m_isMovement;
    bool m_isLifespan;
    bool m_isCollision;
    bool m_isSpawning;
    bool m_isRendering;
    bool m_isGUI;
    bool m_isManualSpawning;
    bool m_GUIDisabledThisFrame;

public:
    Game(const std::string& configFilePath, const std::string& highScoreFilePath);
    ~Game();

    void init(const std::string& config);

    void spawnPlayer();
    void spawnBullet(Vec2 dest);
    void spawnSmallEnemies(std::shared_ptr<Entity> src);
    void spawnBlackHole(bool isMoving); // spawn static or wandering

    bool isCollision(std::shared_ptr<Entity> e1, std::shared_ptr<Entity> e2);

    // systems
    void sPlayer();
    void sRender();
    void sEnemySpawner();
    void sMovement();
    void sUserInput();
    void sCollision();
    void sGUI();
    void sScore();
    void sLifespan();
    void sSpecialAbility();
    void sGravity();

    void run();
};