#pragma once
#include <SFML/Graphics.hpp>
#include "Scene.h"
#include <vector>
#include <memory>
#include <string>

class App;

class SnakeScene : public Scene {
private:
    App* app;

    enum class Difficulty { SELECT, EASY, MEDIUM, HARD };
    Difficulty difficulty = Difficulty::SELECT;

    const int   gridSize = 20;
    const float tileSize = 28.f;

    std::vector<sf::Vector2i> snake;
    sf::Vector2i direction;
    sf::Vector2i nextDirection;
    sf::Vector2i food;

    std::vector<sf::Vector2i> obstacles;

    sf::Vector2i powerUp;
    bool  powerActive = false;
    float powerTimer  = 0.f;
    float powerSpawn  = 0.f;

    int   score      = 0;
    int   highScore  = 0;
    int   level      = 1;
    int   linesEaten = 0;

    float moveDelay     = 0.22f;
    float baseMoveDelay = 0.22f;
    float moveTimer     = 0.f;

    bool paused   = false;
    bool gameOver = false;

    float offsetX = 0.f;
    float offsetY = 0.f;

    sf::Font    font;
    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    std::unique_ptr<sf::Text> scoreText;
    std::unique_ptr<sf::Text> highScoreText;
    std::unique_ptr<sf::Text> levelText;
    std::unique_ptr<sf::Text> pauseText;
    std::unique_ptr<sf::Text> gameOverText;
    std::unique_ptr<sf::Text> infoText;
    std::unique_ptr<sf::Text> obstacleLabel;

    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> easyText;
    std::unique_ptr<sf::Text> mediumText;
    std::unique_ptr<sf::Text> hardText;
    std::unique_ptr<sf::Text> selectPrompt;

    sf::Clock deltaClock;

    // ── Obstacle shape system ─────────────────────────────────────────────────
    struct ShapePattern {
        std::string              label;
        std::vector<sf::Vector2i> cells;  // local coordinates
    };

    std::vector<ShapePattern> allPatterns;
    int currentPatternIndex = 0;

    void buildPatterns();
    std::vector<sf::Vector2i> patternToGrid(const ShapePattern& p, int cx, int cy);

    void spawnFood();
    void spawnObstaclesForLevel();
    void spawnPowerUp();
    void reset();
    void setDifficulty(Difficulty d);

public:
    SnakeScene(App* app);
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;
};
