#pragma once
#include <SFML/Graphics.hpp>
#include "Scene.h"
#include <array>
#include <vector>
#include <memory>

class App;

struct TetrisPiece {
    std::vector<sf::Vector2i> blocks;
    int type = 0;
};

class TetrisScene : public Scene {
private:
    App* app;

    // Board dimensions
    static constexpr int width    = 10;
    static constexpr int height   = 20;
    static constexpr float tileSize = 32.f;

    float offsetX = 0.f;
    float offsetY = 0.f;

    // 7 standard tetrominoes encoded as 4 cells on a 4×4 grid (indices 0-15)
    // Row-major: index = row*4 + col
    int figures[7][4] = {
        {1, 3, 5, 7},   // I  (vertical)
        {2, 4, 5, 7},   // S
        {3, 5, 4, 6},   // Z
        {3, 5, 4, 7},   // T
        {2, 3, 5, 7},   // L
        {3, 5, 7, 6},   // J
        {2, 3, 4, 5}    // O  (square)
    };

    // Per-piece retro colours
    sf::Color pieceColors[7] = {
        sf::Color(0,   240, 240),   // I – cyan
        sf::Color(0,   240, 0),     // S – green
        sf::Color(240, 0,   0),     // Z – red
        sf::Color(160, 0,   240),   // T – purple
        sf::Color(240, 160, 0),     // L – orange
        sf::Color(0,   0,   240),   // J – blue
        sf::Color(240, 240, 0),     // O – yellow
    };

    // Grid: 0 = empty, 1-7 = piece colour index+1
    int grid[height][width] = {};

    TetrisPiece currentPiece;
    TetrisPiece nextPiece;     // preview

    float baseDelay = 0.6f;
    float delay     = 0.6f;
    float timer     = 0.f;

    sf::Clock deltaClock;

    int score    = 0;
    int lines    = 0;
    int level    = 1;
    bool gameOver = false;

    sf::Font font;
    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    std::unique_ptr<sf::Text> scoreText;
    std::unique_ptr<sf::Text> linesText;
    std::unique_ptr<sf::Text> levelText;
    std::unique_ptr<sf::Text> gameOverText;
    std::unique_ptr<sf::Text> nextLabel;
    std::unique_ptr<sf::Text> infoText;

    void spawnPiece();
    bool checkCollision(const TetrisPiece& piece);
    void lockPiece();
    void clearLines();
    void rotatePiece();
    void resetGame();

public:
    TetrisScene(App* app);
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;
};
