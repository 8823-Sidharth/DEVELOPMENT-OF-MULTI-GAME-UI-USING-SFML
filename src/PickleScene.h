#pragma once
#include <SFML/Graphics.hpp>
#include "Scene.h"
#include <vector>
#include <memory>

class App;

// ─── Power capsule types ──────────────────────────────────────────────────────
enum class PowerType {
    EXPAND_PADDLE,   // timed  — widens paddle
    DOUBLE_SCORE,    // timed  — 2× points
    SPEED_BOOST,     // timed  — faster ball
    EXTRA_BALL       // permanent — spawns a bonus ball
};

struct PowerCapsule {
    sf::CircleShape shape;
    sf::Vector2f    velocity;
    PowerType       type;
    bool            alive = true;
};

// ─── Ball entry ───────────────────────────────────────────────────────────────
struct BallEntry {
    sf::CircleShape shape;
    sf::Vector2f    velocity;
    bool            isOriginal = false; // only original ball costs a life
    bool            active     = false; // false = sticking to paddle pre-launch
};

// ─── Brick ───────────────────────────────────────────────────────────────────
struct Brick {
    sf::RectangleShape shape;
    sf::Color          color;
    int                hits  = 1;
    bool               alive = true;
};

class PickleScene : public Scene {
private:
    App* app;

    static constexpr int   BRICK_ROWS = 5;
    static constexpr int   BRICK_COLS = 11;
    static constexpr float boardWidth  = 700.f;
    static constexpr float boardHeight = 560.f;

    float offsetX = 0.f;
    float offsetY = 0.f;

    sf::Font    font;
    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::RectangleShape paddle;
    float paddleBaseWidth = 100.f;
    float paddleCurrentWidth = 100.f;

    std::vector<BallEntry>     balls;
    std::vector<Brick>         bricks;
    std::vector<PowerCapsule>  capsules;

    int score     = 0;
    int highScore = 0;
    int lives     = 3;
    bool gameOver = false;
    bool victory  = false;
    int  scoreMultiplier = 1; // 1 or 2 (double-score power-up)

    // Timed power-up durations
    float expandTimer      = 0.f;   // >0 means active
    float doubleScoreTimer = 0.f;
    float speedBoostTimer  = 0.f;
    float ballBaseSpeed    = 280.f;

    sf::Clock deltaClock;

    // UI
    std::unique_ptr<sf::Text> scoreText;
    std::unique_ptr<sf::Text> highScoreText;
    std::unique_ptr<sf::Text> livesText;
    std::unique_ptr<sf::Text> gameOverText;
    std::unique_ptr<sf::Text> infoText;
    std::unique_ptr<sf::Text> powerStatusText; // shows active effects

    void buildBricks();
    void reset();
    void loseLife();
    void spawnCapsule(sf::Vector2f pos);
    void applyPower(PowerType t);
    void spawnExtraBall();
    void updatePowerTimers(float dt);
    void updatePaddleWidth();

public:
    PickleScene(App* app);
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;
};
