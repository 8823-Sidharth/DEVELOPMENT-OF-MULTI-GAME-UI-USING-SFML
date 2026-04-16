#include "SnakeScene.h"
#include "App.h"
#include "MenuScene.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <cmath>

// ─── Retro colour palette ─────────────────────────────────────────────────────
static const sf::Color C_BG        = sf::Color(10,  12,  20);
static const sf::Color C_BOARD     = sf::Color(15,  18,  35, 220);
static const sf::Color C_BORDER    = sf::Color(0,   255, 180);
static const sf::Color C_SNAKE_H   = sf::Color(0,   255, 120);
static const sf::Color C_SNAKE_B   = sf::Color(0,   180, 80);
static const sf::Color C_FOOD      = sf::Color(255, 80,  80);
static const sf::Color C_POWER     = sf::Color(255, 220, 0);
static const sf::Color C_OBSTACLE  = sf::Color(60,  80,  200);
static const sf::Color C_OBS_OUT   = sf::Color(140, 160, 255);
static const sf::Color C_TEXT      = sf::Color(0,   255, 180);
static const sf::Color C_TITLE     = sf::Color(255, 220, 0);
static const sf::Color C_EASY      = sf::Color(0,   255, 120);
static const sf::Color C_MEDIUM    = sf::Color(255, 180, 0);
static const sf::Color C_HARD      = sf::Color(255, 60,  60);

// ─── Safe-zone helper ────────────────────────────────────────────────────────
// Keeps a clear radius around the snake spawn point (board centre) so
// obstacles NEVER overlap the starting position.
static const int SAFE_RADIUS = 4;

static bool isSafe(int x, int y, int gridSize)
{
    int cx = gridSize / 2, cy = gridSize / 2;
    int dx = x - cx,       dy = y - cy;
    return (dx * dx + dy * dy) <= SAFE_RADIUS * SAFE_RADIUS;
}

// ─── buildPatterns ────────────────────────────────────────────────────────────
void SnakeScene::buildPatterns()
{
    allPatterns.clear();

    auto dedup = [](std::vector<sf::Vector2i>& v)
    {
        std::sort(v.begin(), v.end(), [](auto& a, auto& b){
            return a.x < b.x || (a.x == b.x && a.y < b.y);
        });
        v.erase(std::unique(v.begin(), v.end(), [](auto& a, auto& b){
            return a.x == b.x && a.y == b.y;
        }), v.end());
    };

    // ── Geometry shapes ───────────────────────────────────────────────────────

    // Hollow square (7x7)
    {
        ShapePattern p; p.label = "SQUARE";
        for (int c = 0; c <= 6; ++c) { p.cells.push_back({c,0}); p.cells.push_back({c,6}); }
        for (int r = 1; r <= 5; ++r) { p.cells.push_back({0,r}); p.cells.push_back({6,r}); }
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // Triangle outline
    {
        ShapePattern p; p.label = "TRIANGLE";
        for (int c = 0; c <= 8; ++c) p.cells.push_back({c, 7});
        for (int r = 0; r <= 7; ++r) p.cells.push_back({r, 7 - r});
        for (int r = 0; r <= 7; ++r) p.cells.push_back({8 - r, 7 - r});
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // Diamond outline
    {
        ShapePattern p; p.label = "DIAMOND";
        int R = 4;
        for (int dr = -R; dr <= R; ++dr)
        {
            int mc = R - std::abs(dr);
            p.cells.push_back({R - mc, R + dr});
            if (mc > 0) p.cells.push_back({R + mc, R + dr});
        }
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // Cross / Plus
    {
        ShapePattern p; p.label = "CROSS";
        for (int c = 0; c <= 6; ++c) p.cells.push_back({c, 3});
        for (int r = 0; r <= 6; ++r) p.cells.push_back({3, r});
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // Diagonal X
    {
        ShapePattern p; p.label = "X-SHAPE";
        for (int i = 0; i <= 6; ++i)
        {
            p.cells.push_back({i, i});
            p.cells.push_back({6 - i, i});
        }
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // Circle (approximate outline)
    {
        ShapePattern p; p.label = "CIRCLE";
        float cx = 4.f, cy = 4.f;
        for (int r = 0; r <= 8; ++r)
            for (int c = 0; c <= 8; ++c)
            {
                float d = std::sqrt((c - cx) * (c - cx) + (r - cy) * (r - cy));
                if (d >= 3.3f && d <= 4.2f) p.cells.push_back({c, r});
            }
        allPatterns.push_back(p);
    }
    // Arrow (right-pointing)
    {
        ShapePattern p; p.label = "ARROW";
        for (int c = 0; c <= 4; ++c) p.cells.push_back({c, 3}); // shaft
        p.cells.push_back({5,1}); p.cells.push_back({6,2}); p.cells.push_back({7,3});
        p.cells.push_back({6,4}); p.cells.push_back({5,5});
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // Zigzag
    {
        ShapePattern p; p.label = "ZIGZAG";
        for (int i = 0; i <= 3; ++i) p.cells.push_back({4 + i, i});
        for (int i = 0; i <= 4; ++i) p.cells.push_back({4 - i, 3 + i});
        for (int i = 0; i <= 3; ++i) p.cells.push_back({i, 7});
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // H-shape
    {
        ShapePattern p; p.label = "H-SHAPE";
        for (int r = 0; r <= 6; ++r) { p.cells.push_back({0,r}); p.cells.push_back({6,r}); }
        for (int c = 1; c <= 5; ++c) p.cells.push_back({c, 3});
        dedup(p.cells);
        allPatterns.push_back(p);
    }
    // U-shape
    {
        ShapePattern p; p.label = "U-SHAPE";
        for (int r = 0; r <= 5; ++r) { p.cells.push_back({0,r}); p.cells.push_back({6,r}); }
        for (int c = 0; c <= 6; ++c) p.cells.push_back({c, 6});
        dedup(p.cells);
        allPatterns.push_back(p);
    }

    // ── Numbers 0–9 ──────────────────────────────────────────────────────────
    auto addNum = [&](const std::string& lbl, std::vector<sf::Vector2i> raw)
    {
        ShapePattern p; p.label = lbl;
        for (auto& c : raw) p.cells.push_back({c.x + 1, c.y});
        allPatterns.push_back(p);
    };

    addNum("NUM:0", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{4,3},{0,4},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addNum("NUM:1", {{2,0},{1,1},{2,1},{2,2},{2,3},{2,4},{2,5},{1,6},{2,6},{3,6}});
    addNum("NUM:2", {{0,0},{1,0},{2,0},{3,0},{4,1},{3,2},{2,3},{1,4},{0,5},{0,6},{1,6},{2,6},{3,6},{4,6}});
    addNum("NUM:3", {{0,0},{1,0},{2,0},{3,0},{4,1},{3,2},{2,2},{3,3},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addNum("NUM:4", {{0,0},{3,0},{0,1},{3,1},{0,2},{3,2},{0,3},{1,3},{2,3},{3,3},{4,3},{3,4},{3,5},{3,6}});
    addNum("NUM:5", {{0,0},{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{1,2},{2,2},{3,2},{4,3},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addNum("NUM:6", {{1,0},{2,0},{3,0},{0,1},{0,2},{0,3},{1,3},{2,3},{3,3},{0,4},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addNum("NUM:7", {{0,0},{1,0},{2,0},{3,0},{4,0},{4,1},{3,2},{2,3},{2,4},{2,5},{2,6}});
    addNum("NUM:8", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{1,3},{2,3},{3,3},{0,4},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addNum("NUM:9", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{1,3},{2,3},{3,3},{4,3},{4,4},{4,5},{1,6},{2,6},{3,6}});

    // ── Alphabet A–Z ─────────────────────────────────────────────────────────
    auto addLtr = [&](const std::string& lbl, std::vector<sf::Vector2i> raw)
    {
        ShapePattern p; p.label = lbl;
        for (auto& c : raw) p.cells.push_back({c.x + 1, c.y});
        allPatterns.push_back(p);
    };

    addLtr("A", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3},{4,3},{0,4},{4,4},{0,5},{4,5},{0,6},{4,6}});
    addLtr("B", {{0,0},{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3},{0,4},{4,4},{0,5},{4,5},{0,6},{1,6},{2,6},{3,6}});
    addLtr("C", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{0,3},{0,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addLtr("D", {{0,0},{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{4,3},{0,4},{4,4},{0,5},{4,5},{0,6},{1,6},{2,6},{3,6}});
    addLtr("E", {{0,0},{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{0,3},{1,3},{2,3},{0,4},{0,5},{0,6},{1,6},{2,6},{3,6},{4,6}});
    addLtr("F", {{0,0},{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{0,3},{1,3},{2,3},{3,3},{0,4},{0,5},{0,6}});
    addLtr("G", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{0,3},{3,3},{4,3},{0,4},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addLtr("H", {{0,0},{4,0},{0,1},{4,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3},{4,3},{0,4},{4,4},{0,5},{4,5},{0,6},{4,6}});
    addLtr("I", {{0,0},{1,0},{2,0},{3,0},{4,0},{2,1},{2,2},{2,3},{2,4},{2,5},{0,6},{1,6},{2,6},{3,6},{4,6}});
    addLtr("J", {{3,0},{4,0},{4,1},{4,2},{4,3},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addLtr("K", {{0,0},{4,0},{0,1},{3,1},{0,2},{2,2},{0,3},{1,3},{0,4},{2,4},{0,5},{3,5},{0,6},{4,6}});
    addLtr("L", {{0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{1,6},{2,6},{3,6},{4,6}});
    addLtr("M", {{0,0},{4,0},{0,1},{1,1},{3,1},{4,1},{0,2},{2,2},{4,2},{0,3},{4,3},{0,4},{4,4},{0,5},{4,5},{0,6},{4,6}});
    addLtr("N", {{0,0},{4,0},{0,1},{1,1},{4,1},{0,2},{2,2},{4,2},{0,3},{3,3},{4,3},{0,4},{4,4},{0,5},{4,5},{0,6},{4,6}});
    addLtr("O", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{4,3},{0,4},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addLtr("P", {{0,0},{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3},{0,4},{0,5},{0,6}});
    addLtr("Q", {{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{4,3},{0,4},{3,4},{4,4},{0,5},{3,5},{1,6},{2,6},{4,6}});
    addLtr("R", {{0,0},{1,0},{2,0},{3,0},{0,1},{4,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3},{0,4},{3,4},{0,5},{4,5},{0,6},{4,6}});
    addLtr("S", {{1,0},{2,0},{3,0},{4,0},{0,1},{0,2},{1,3},{2,3},{3,3},{4,4},{4,5},{0,6},{1,6},{2,6},{3,6}});
    addLtr("T", {{0,0},{1,0},{2,0},{3,0},{4,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6}});
    addLtr("U", {{0,0},{4,0},{0,1},{4,1},{0,2},{4,2},{0,3},{4,3},{0,4},{4,4},{0,5},{4,5},{1,6},{2,6},{3,6}});
    addLtr("V", {{0,0},{4,0},{0,1},{4,1},{1,2},{3,2},{1,3},{3,3},{2,4},{2,5},{2,6}});
    addLtr("W", {{0,0},{4,0},{0,1},{4,1},{0,2},{2,2},{4,2},{0,3},{2,3},{4,3},{1,4},{3,4},{1,5},{3,5},{0,6},{4,6}});
    addLtr("X", {{0,0},{4,0},{1,1},{3,1},{2,2},{2,3},{1,4},{3,4},{0,5},{4,5},{0,6},{4,6}});
    addLtr("Y", {{0,0},{4,0},{1,1},{3,1},{2,2},{2,3},{2,4},{2,5},{2,6}});
    addLtr("Z", {{0,0},{1,0},{2,0},{3,0},{4,0},{4,1},{3,2},{2,3},{1,4},{0,5},{0,6},{1,6},{2,6},{3,6},{4,6}});
}

// ─── patternToGrid ────────────────────────────────────────────────────────────
std::vector<sf::Vector2i> SnakeScene::patternToGrid(const ShapePattern& p, int cx, int cy)
{
    int minX = 99, minY = 99, maxX = -1, maxY = -1;
    for (auto& c : p.cells)
    {
        minX = std::min(minX, c.x); minY = std::min(minY, c.y);
        maxX = std::max(maxX, c.x); maxY = std::max(maxY, c.y);
    }
    int offX = cx - (minX + maxX) / 2;
    int offY = cy - (minY + maxY) / 2;

    std::vector<sf::Vector2i> result;
    for (auto& c : p.cells)
    {
        sf::Vector2i g = { c.x + offX, c.y + offY };
        if (g.x < 1 || g.x >= gridSize - 1 || g.y < 1 || g.y >= gridSize - 1) continue;
        if (isSafe(g.x, g.y, gridSize)) continue;   // ← never overlaps spawn
        result.push_back(g);
    }
    return result;
}

// ─── Constructor ─────────────────────────────────────────────────────────────
SnakeScene::SnakeScene(App* app)
: app(app)
{
    std::srand((unsigned)std::time(nullptr));

    auto size = app->getWindow().getSize();
    offsetX = (size.x - gridSize * tileSize) / 2.f;
    offsetY = (size.y - gridSize * tileSize) / 2.f;

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        std::cout << "Font failed\n";
    if (!bgTexture.loadFromFile("assets/images/snake_retro.png"))
        std::cout << "BG texture failed\n";

    bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    bgSprite->setScale({
        (float)size.x / (bgTexture.getSize().x > 0 ? bgTexture.getSize().x : 1),
        (float)size.y / (bgTexture.getSize().y > 0 ? bgTexture.getSize().y : 1)
    });

    buildPatterns();

    auto makeText = [&](const std::string& str, unsigned sz, sf::Color col)
        -> std::unique_ptr<sf::Text>
    {
        auto t = std::make_unique<sf::Text>(font, str, sz);
        t->setFillColor(col);
        return t;
    };

    titleText     = makeText("SNAKE",               72, C_TITLE);
    selectPrompt  = makeText("SELECT DIFFICULTY",   28, C_TEXT);
    easyText      = makeText("[ EASY ]",            36, C_EASY);
    mediumText    = makeText("[ MEDIUM ]",          36, C_MEDIUM);
    hardText      = makeText("[ HARD ]",            36, C_HARD);
    scoreText     = makeText("SCORE: 0",            24, C_TEXT);
    highScoreText = makeText("BEST:  0",            24, C_TEXT);
    levelText     = makeText("LEVEL: 1",            24, C_TEXT);
    pauseText     = makeText("-- PAUSED --",        40, C_TITLE);
    gameOverText  = makeText("GAME OVER",           56, C_HARD);
    infoText      = makeText("B=MENU  P=PAUSE  WASD/ARROWS=Move", 18, sf::Color(120,120,160));
    obstacleLabel = makeText("",                    22, sf::Color(200,200,255));

    float cx2 = (float)size.x / 2.f;
    titleText->setPosition({cx2 - titleText->getGlobalBounds().size.x / 2.f, 130.f});
    selectPrompt->setPosition({cx2 - selectPrompt->getGlobalBounds().size.x / 2.f, 240.f});
    easyText->setPosition({cx2 - easyText->getGlobalBounds().size.x / 2.f,   310.f});
    mediumText->setPosition({cx2 - mediumText->getGlobalBounds().size.x / 2.f, 370.f});
    hardText->setPosition({cx2 - hardText->getGlobalBounds().size.x / 2.f,   430.f});

    scoreText->setPosition({offsetX, offsetY - 60.f});
    highScoreText->setPosition({offsetX, offsetY - 34.f});
    levelText->setPosition({offsetX + gridSize * tileSize - 130.f, offsetY - 60.f});
    infoText->setPosition({offsetX, offsetY + gridSize * tileSize + 8.f});
    obstacleLabel->setPosition({offsetX + gridSize * tileSize - 210.f, offsetY - 34.f});

    float midX = cx2 - pauseText->getGlobalBounds().size.x / 2.f;
    float midY = (float)size.y / 2.f - 30.f;
    pauseText->setPosition({midX, midY});
    gameOverText->setPosition({cx2 - gameOverText->getGlobalBounds().size.x / 2.f, midY - 20.f});
}

// ─── spawnFood ───────────────────────────────────────────────────────────────
void SnakeScene::spawnFood()
{
    int tries = 0;
    do {
        food = { std::rand() % gridSize, std::rand() % gridSize };
        tries++;
    } while (tries < 500 && (
        std::find(snake.begin(),     snake.end(),     food) != snake.end() ||
        std::find(obstacles.begin(), obstacles.end(), food) != obstacles.end()
    ));
}

// ─── spawnObstaclesForLevel ───────────────────────────────────────────────────
void SnakeScene::spawnObstaclesForLevel()
{
    obstacles.clear();
    obstacleLabel->setString("");

    // Easy: no obstacles for first 2 levels
    if (difficulty == Difficulty::EASY && level <= 2) return;

    // Pick pattern (cycle through all)
    currentPatternIndex = (level - 1) % (int)allPatterns.size();
    const ShapePattern& pat = allPatterns[currentPatternIndex];

    // Place shape in one of four corner quadrants — rotates each level.
    // This guarantees the shape is NEVER centred on the snake spawn point.
    struct Anchor { int x, y; };
    Anchor anchors[4] = {
        { gridSize / 4,         gridSize / 4     },   // top-left
        { gridSize * 3 / 4,     gridSize / 4     },   // top-right
        { gridSize / 4,         gridSize * 3 / 4 },   // bottom-left
        { gridSize * 3 / 4,     gridSize * 3 / 4 }    // bottom-right
    };
    Anchor a = anchors[(level - 1) % 4];

    auto cells = patternToGrid(pat, a.x, a.y);

    // Strip any cell that somehow landed on the snake
    cells.erase(std::remove_if(cells.begin(), cells.end(), [&](const sf::Vector2i& c){
        return std::find(snake.begin(), snake.end(), c) != snake.end();
    }), cells.end());

    obstacles = cells;
    obstacleLabel->setString("SHAPE: " + pat.label);

    // Extra random tiles — few and always outside safe zone
    // Medium: 0 extras on level 1, +1 per level, max 4
    // Hard:   +1 per level starting level 1, max 7
    int maxExtra = 0;
    if      (difficulty == Difficulty::MEDIUM) maxExtra = std::min(4, level - 1);
    else if (difficulty == Difficulty::HARD)   maxExtra = std::min(7, level);

    for (int i = 0; i < maxExtra; ++i)
    {
        sf::Vector2i ob;
        int tries = 0;
        do {
            ob = { 1 + std::rand() % (gridSize - 2), 1 + std::rand() % (gridSize - 2) };
            tries++;
        } while (tries < 300 && (
            isSafe(ob.x, ob.y, gridSize) ||
            std::find(snake.begin(),     snake.end(),     ob) != snake.end() ||
            std::find(obstacles.begin(), obstacles.end(), ob) != obstacles.end() ||
            ob == food
        ));
        if (tries < 300) obstacles.push_back(ob);
    }
}

// ─── spawnPowerUp ─────────────────────────────────────────────────────────────
void SnakeScene::spawnPowerUp()
{
    int tries = 0;
    do {
        powerUp = { 1 + std::rand() % (gridSize - 2), 1 + std::rand() % (gridSize - 2) };
        tries++;
    } while (tries < 300 && (
        std::find(snake.begin(),     snake.end(),     powerUp) != snake.end() ||
        std::find(obstacles.begin(), obstacles.end(), powerUp) != obstacles.end() ||
        powerUp == food
    ));
    powerActive = true;
    powerTimer  = 10.f;
}

// ─── setDifficulty ───────────────────────────────────────────────────────────
// Speeds tuned for comfortable play at all levels.
// Per-level formula: delay × 0.93^(level-1), floored so it never becomes unplayable.
//   Easy   floor = 0.12s  (~8 steps/sec max)
//   Medium floor = 0.08s  (~12 steps/sec max)
//   Hard   floor = 0.055s (~18 steps/sec max)
void SnakeScene::setDifficulty(Difficulty d)
{
    difficulty = d;
    switch (d)
    {
        case Difficulty::EASY:   baseMoveDelay = 0.22f; break;
        case Difficulty::MEDIUM: baseMoveDelay = 0.15f; break;
        case Difficulty::HARD:   baseMoveDelay = 0.11f; break;
        default: break;
    }
    moveDelay = baseMoveDelay;
    reset();
}

// ─── reset ───────────────────────────────────────────────────────────────────
void SnakeScene::reset()
{
    snake.clear();
    int sx = gridSize / 2, sy = gridSize / 2;
    snake.push_back({ sx,     sy });
    snake.push_back({ sx - 1, sy });
    snake.push_back({ sx - 2, sy });

    direction     = { 1, 0 };
    nextDirection = { 1, 0 };
    score         = 0;
    level         = 1;
    linesEaten    = 0;
    moveTimer     = 0.f;
    moveDelay     = baseMoveDelay;
    paused        = false;
    gameOver      = false;
    powerActive   = false;
    powerSpawn    = 8.f;
    currentPatternIndex = 0;

    spawnFood();
    spawnObstaclesForLevel();  // safe zone enforced inside

    scoreText->setString("SCORE: 0");
    levelText->setString("LEVEL: 1");
}

// ─── handleEvent ─────────────────────────────────────────────────────────────
void SnakeScene::handleEvent(const sf::Event& event)
{
    if (!event.is<sf::Event::KeyPressed>()) return;
    auto key = event.getIf<sf::Event::KeyPressed>()->code;

    if (difficulty == Difficulty::SELECT)
    {
        if (key == sf::Keyboard::Key::E) setDifficulty(Difficulty::EASY);
        if (key == sf::Keyboard::Key::M) setDifficulty(Difficulty::MEDIUM);
        if (key == sf::Keyboard::Key::H) setDifficulty(Difficulty::HARD);
        if (key == sf::Keyboard::Key::B) app->changeScene(std::make_unique<MenuScene>(app));
        return;
    }

    if (gameOver)
    {
        if (key == sf::Keyboard::Key::R) reset();
        if (key == sf::Keyboard::Key::B) app->changeScene(std::make_unique<MenuScene>(app));
        return;
    }

    if (key == sf::Keyboard::Key::B) app->changeScene(std::make_unique<MenuScene>(app));
    if (key == sf::Keyboard::Key::P) paused = !paused;

    if (!paused)
    {
        using K = sf::Keyboard::Key;
        if ((key == K::Up    || key == K::W) && direction.y == 0) nextDirection = { 0,-1};
        if ((key == K::Down  || key == K::S) && direction.y == 0) nextDirection = { 0, 1};
        if ((key == K::Left  || key == K::A) && direction.x == 0) nextDirection = {-1, 0};
        if ((key == K::Right || key == K::D) && direction.x == 0) nextDirection = { 1, 0};
    }
}

// ─── update ──────────────────────────────────────────────────────────────────
void SnakeScene::update()
{
    if (difficulty == Difficulty::SELECT || gameOver || paused) return;

    float dt = deltaClock.restart().asSeconds();
    if (dt > 0.2f) dt = 0.2f;  // guard against window-drag spikes

    // Power-up timer
    if (powerActive)
    {
        powerTimer -= dt;
        if (powerTimer <= 0.f) powerActive = false;
    }
    else
    {
        powerSpawn -= dt;
        if (powerSpawn <= 0.f)
        {
            spawnPowerUp();
            powerSpawn = 12.f + (std::rand() % 8);
        }
    }

    moveTimer += dt;
    if (moveTimer < moveDelay) return;
    moveTimer = 0.f;

    direction = nextDirection;
    sf::Vector2i newHead = snake.front() + direction;

    // Wall collision
    if (newHead.x < 0 || newHead.x >= gridSize ||
        newHead.y < 0 || newHead.y >= gridSize)
    {
        gameOver = true;
        if (score > highScore) { highScore = score; highScoreText->setString("BEST: " + std::to_string(highScore)); }
        gameOverText->setString("GAME OVER  R=Restart  B=Menu");
        return;
    }

    // Self collision (skip tail tip — it moves away this step)
    for (int i = 0; i < (int)snake.size() - 1; ++i)
    {
        if (snake[i] == newHead)
        {
            gameOver = true;
            if (score > highScore) { highScore = score; highScoreText->setString("BEST: " + std::to_string(highScore)); }
            gameOverText->setString("GAME OVER  R=Restart  B=Menu");
            return;
        }
    }

    // Obstacle collision
    if (std::find(obstacles.begin(), obstacles.end(), newHead) != obstacles.end())
    {
        gameOver = true;
        if (score > highScore) { highScore = score; highScoreText->setString("BEST: " + std::to_string(highScore)); }
        gameOverText->setString("GAME OVER  R=Restart  B=Menu");
        return;
    }

    snake.insert(snake.begin(), newHead);

    if (newHead == food)
    {
        score += 10 * level;
        linesEaten++;
        spawnFood();

        if (linesEaten % 5 == 0)
        {
            level++;
            // Gentle 7% speed-up per level with per-difficulty floor
            float floor = (difficulty == Difficulty::HARD)   ? 0.055f :
                          (difficulty == Difficulty::MEDIUM)  ? 0.08f  : 0.12f;
            moveDelay = std::max(floor, baseMoveDelay * std::pow(0.93f, (float)(level - 1)));
            spawnObstaclesForLevel();
            levelText->setString("LEVEL: " + std::to_string(level));
        }
        scoreText->setString("SCORE: " + std::to_string(score));
        // tail kept → snake grows
    }
    else if (powerActive && newHead == powerUp)
    {
        score += 50 * level;
        scoreText->setString("SCORE: " + std::to_string(score));
        powerActive = false;
        snake.push_back(snake.back());
        snake.push_back(snake.back());
    }
    else
    {
        snake.pop_back();
    }
}

// ─── render ──────────────────────────────────────────────────────────────────
void SnakeScene::render(sf::RenderWindow& window)
{
    auto size = app->getWindow().getSize();

    if (bgTexture.getSize().x > 0)
        window.draw(*bgSprite);

    sf::RectangleShape overlay({(float)size.x, (float)size.y});
    overlay.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(overlay);

    // ── Difficulty select ─────────────────────────────────────────────────────
    if (difficulty == Difficulty::SELECT)
    {
        window.draw(*titleText);
        window.draw(*selectPrompt);

        auto drawBtn = [&](sf::Text& txt, sf::Color col)
        {
            auto b = txt.getGlobalBounds();
            sf::RectangleShape box({b.size.x + 30.f, b.size.y + 18.f});
            box.setPosition({b.position.x - 15.f, b.position.y - 6.f});
            box.setFillColor(sf::Color(col.r, col.g, col.b, 30));
            box.setOutlineThickness(2.f);
            box.setOutlineColor(col);
            window.draw(box);
            window.draw(txt);
        };

        drawBtn(*easyText,   C_EASY);
        drawBtn(*mediumText, C_MEDIUM);
        drawBtn(*hardText,   C_HARD);

        float cx2 = (float)size.x / 2.f;
        auto makeHint = [&](const std::string& s, sf::Color col, float y) {
            sf::Text t(font, s, 18);
            t.setFillColor(col);
            t.setPosition({cx2 - t.getGlobalBounds().size.x / 2.f, y});
            window.draw(t);
        };
        makeHint("EASY   — No obstacles for first 2 levels, gentle speed increase", C_EASY,   490.f);
        makeHint("MEDIUM — Shapes from level 1, moderate speed & extra tiles",      C_MEDIUM, 513.f);
        makeHint("HARD   — Shapes + extra tiles from level 1, faster speed",        C_HARD,   536.f);

        sf::Text hint(font, "E = Easy    M = Medium    H = Hard    B = Back", 20);
        hint.setFillColor(sf::Color(120, 120, 160));
        hint.setPosition({cx2 - hint.getGlobalBounds().size.x / 2.f, 570.f});
        window.draw(hint);
        return;
    }

    // ── Board ─────────────────────────────────────────────────────────────────
    sf::RectangleShape board({gridSize * tileSize, gridSize * tileSize});
    board.setPosition({offsetX, offsetY});
    board.setFillColor(C_BOARD);
    board.setOutlineThickness(3.f);
    board.setOutlineColor(C_BORDER);
    window.draw(board);

    // Grid lines
    sf::RectangleShape gridLine;
    gridLine.setFillColor(sf::Color(255, 255, 255, 8));
    for (int i = 1; i < gridSize; ++i)
    {
        gridLine.setSize({gridSize * tileSize, 1.f});
        gridLine.setPosition({offsetX, offsetY + i * tileSize});
        window.draw(gridLine);
        gridLine.setSize({1.f, gridSize * tileSize});
        gridLine.setPosition({offsetX + i * tileSize, offsetY});
        window.draw(gridLine);
    }

    sf::RectangleShape tile({tileSize - 2.f, tileSize - 2.f});

    // Obstacles
    for (auto& ob : obstacles)
    {
        tile.setFillColor(C_OBSTACLE);
        tile.setOutlineThickness(1.5f);
        tile.setOutlineColor(C_OBS_OUT);
        tile.setPosition({offsetX + ob.x * tileSize + 1.f, offsetY + ob.y * tileSize + 1.f});
        window.draw(tile);
    }
    tile.setOutlineThickness(0.f);

    // Snake
    for (int i = (int)snake.size() - 1; i >= 0; --i)
    {
        float fade = 1.f - (float)i / (float)snake.size() * 0.55f;
        sf::Color col(
            (std::uint8_t)(C_SNAKE_B.r * fade),
            (std::uint8_t)(C_SNAKE_B.g * fade),
            (std::uint8_t)(C_SNAKE_B.b * fade)
        );
        if (i == 0) col = C_SNAKE_H;
        tile.setFillColor(col);
        tile.setPosition({
            offsetX + snake[i].x * tileSize + 1.f,
            offsetY + snake[i].y * tileSize + 1.f
        });
        window.draw(tile);
    }

    // Food
    tile.setFillColor(C_FOOD);
    tile.setPosition({offsetX + food.x * tileSize + 1.f, offsetY + food.y * tileSize + 1.f});
    window.draw(tile);

    // Power-up (flashes last 4 seconds)
    if (powerActive)
    {
        tile.setFillColor(C_POWER);
        tile.setPosition({
            offsetX + powerUp.x * tileSize + 1.f,
            offsetY + powerUp.y * tileSize + 1.f
        });
        window.draw(tile);
        if (powerTimer < 4.f && (int)(powerTimer * 5) % 2 == 0)
        {
            tile.setOutlineThickness(2.f);
            tile.setOutlineColor(sf::Color::White);
            window.draw(tile);
            tile.setOutlineThickness(0.f);
        }
    }

    // HUD
    window.draw(*scoreText);
    window.draw(*highScoreText);
    window.draw(*levelText);
    window.draw(*infoText);
    window.draw(*obstacleLabel);

    // Speed bar (bottom-right of board)
    {
        float maxD = baseMoveDelay;
        float minD = (difficulty == Difficulty::HARD)   ? 0.055f :
                     (difficulty == Difficulty::MEDIUM)  ? 0.08f  : 0.12f;
        float pct  = 1.f - (moveDelay - minD) / std::max(0.001f, maxD - minD);
        pct = std::max(0.f, std::min(1.f, pct));

        float barW = 100.f;
        float barX = offsetX + gridSize * tileSize - barW;
        float barY = offsetY + gridSize * tileSize + 10.f;

        sf::RectangleShape barBg({barW, 8.f});
        barBg.setPosition({barX, barY});
        barBg.setFillColor(sf::Color(40, 40, 60));
        window.draw(barBg);

        sf::RectangleShape barFill({barW * pct, 8.f});
        barFill.setPosition({barX, barY});
        sf::Color spColor = (pct < 0.5f)
            ? sf::Color((std::uint8_t)(pct * 2 * 255), 200, 0)
            : sf::Color(255, (std::uint8_t)((1.f - pct) * 2 * 200), 0);
        barFill.setFillColor(spColor);
        window.draw(barFill);

        sf::Text spLabel(font, "SPD", 14);
        spLabel.setFillColor(sf::Color(160, 160, 200));
        spLabel.setPosition({barX - 36.f, barY - 2.f});
        window.draw(spLabel);
    }

    // Pause overlay
    if (paused)
    {
        sf::RectangleShape dim({(float)size.x, (float)size.y});
        dim.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(dim);
        window.draw(*pauseText);
    }

    // Game-over overlay
    if (gameOver)
    {
        sf::RectangleShape dim({(float)size.x, (float)size.y});
        dim.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(dim);
        window.draw(*gameOverText);

        sf::Text sub(font, "Score: " + std::to_string(score) +
                           "   Best: " + std::to_string(highScore), 28);
        sub.setFillColor(C_TEXT);
        sub.setPosition({
            (float)size.x / 2.f - sub.getGlobalBounds().size.x / 2.f,
            (float)size.y / 2.f + 40.f
        });
        window.draw(sub);
    }
}
