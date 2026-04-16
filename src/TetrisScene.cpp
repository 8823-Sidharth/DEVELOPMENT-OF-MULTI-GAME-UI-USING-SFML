#include "TetrisScene.h"
#include "App.h"
#include "MenuScene.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

// ─── Retro palette ────────────────────────────────────────────────────────────
static const sf::Color C_BG     = sf::Color(10,  10,  25);
static const sf::Color C_BOARD  = sf::Color(12,  12,  35, 230);
static const sf::Color C_BORDER = sf::Color(0,   200, 255);
static const sf::Color C_GRID   = sf::Color(255, 255, 255, 12);
static const sf::Color C_TEXT   = sf::Color(0,   220, 255);
static const sf::Color C_TITLE  = sf::Color(255, 220, 0);
static const sf::Color C_GAMEOVER = sf::Color(255, 60, 60);
// ─────────────────────────────────────────────────────────────────────────────

TetrisScene::TetrisScene(App* app)
: app(app)
{
    std::srand((unsigned)std::time(nullptr));

    auto size = app->getWindow().getSize();
    offsetX = (size.x - width  * tileSize) / 2.f - 80.f; // shift left to leave room for sidebar
    offsetY = (size.y - height * tileSize) / 2.f;

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        std::cout << "Font failed\n";

    if (!bgTexture.loadFromFile("assets/images/tetris_retro.png"))
        std::cout << "BG texture failed\n";

    bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    bgSprite->setScale({
        (float)size.x / (bgTexture.getSize().x > 0 ? bgTexture.getSize().x : 1),
        (float)size.y / (bgTexture.getSize().y > 0 ? bgTexture.getSize().y : 1)
    });

    auto makeText = [&](const std::string& s, unsigned sz, sf::Color col)
        -> std::unique_ptr<sf::Text>
    {
        auto t = std::make_unique<sf::Text>(font, s, sz);
        t->setFillColor(col);
        return t;
    };

    float sideX = offsetX + width * tileSize + 30.f;

    scoreText   = makeText("SCORE\n0",     22, C_TEXT);
    linesText   = makeText("LINES\n0",     22, C_TEXT);
    levelText   = makeText("LEVEL\n1",     22, C_TEXT);
    nextLabel   = makeText("NEXT",         22, C_TEXT);
    gameOverText= makeText("GAME OVER\nR=Restart  B=Menu", 32, C_GAMEOVER);
    infoText    = makeText("LEFT/RIGHT  UP=Rotate\nDOWN=Drop  B=Menu", 18,
                           sf::Color(100,100,150));

    scoreText->setPosition({sideX, offsetY});
    linesText->setPosition({sideX, offsetY + 80.f});
    levelText->setPosition({sideX, offsetY + 160.f});
    nextLabel->setPosition({sideX, offsetY + 260.f});
    infoText->setPosition({offsetX, offsetY + height * tileSize + 10.f});

    float goX = (float)size.x / 2.f - 160.f;
    float goY = (float)size.y / 2.f - 50.f;
    gameOverText->setPosition({goX, goY});

    // Seed both pieces
    spawnPiece(); // fills nextPiece first call acts as "current"
    spawnPiece(); // now currentPiece is set, nextPiece queued
}

// ─── spawnPiece ──────────────────────────────────────────────────────────────
void TetrisScene::spawnPiece()
{
    currentPiece = nextPiece; // promote queued piece

    // Build a new next piece
    int n = std::rand() % 7;
    nextPiece.blocks.clear();
    nextPiece.type = n;

    for (int i = 0; i < 4; i++)
    {
        int idx = figures[n][i];
        nextPiece.blocks.push_back({
            idx % 4 + width / 2 - 2,   // centre horizontally on board
            idx / 4                     // row within 4-row grid
        });
    }

    // On first call (constructor) currentPiece is empty; just copy
    if (currentPiece.blocks.empty())
        currentPiece = nextPiece;
}

// ─── checkCollision ───────────────────────────────────────────────────────────
bool TetrisScene::checkCollision(const TetrisPiece& piece)
{
    for (auto& b : piece.blocks)
    {
        if (b.x < 0 || b.x >= width || b.y >= height)
            return true;
        if (b.y >= 0 && grid[b.y][b.x])
            return true;
    }
    return false;
}

// ─── lockPiece ───────────────────────────────────────────────────────────────
void TetrisScene::lockPiece()
{
    for (auto& b : currentPiece.blocks)
        if (b.y >= 0)
            grid[b.y][b.x] = currentPiece.type + 1; // store colour index (1-7)

    clearLines();

    spawnPiece();

    // Game-over: new piece immediately collides
    if (checkCollision(currentPiece))
        gameOver = true;
}

// ─── clearLines ──────────────────────────────────────────────────────────────
// BUG FIX: original code incremented i after clearing, skipping the shifted row.
// We must re-check the same row index after a clear (decrement i).
void TetrisScene::clearLines()
{
    int cleared = 0;
    for (int i = height - 1; i >= 0; )   // ← note: no i-- in loop header
    {
        bool full = true;
        for (int j = 0; j < width; j++)
            if (!grid[i][j]) { full = false; break; }

        if (full)
        {
            // Shift every row above down by one
            for (int k = i; k > 0; k--)
                for (int j = 0; j < width; j++)
                    grid[k][j] = grid[k-1][j];

            // Clear top row
            for (int j = 0; j < width; j++)
                grid[0][j] = 0;

            cleared++;
            // Do NOT decrement i – the row that fell into position i needs re-checking
        }
        else
        {
            i--;   // only move up if this row was not cleared
        }
    }

    if (cleared == 0) return;

    // Scoring (classic Tetris points)
    static const int pts[5] = {0, 100, 300, 500, 800};
    int bonus = (cleared <= 4) ? pts[cleared] : 800;
    score += bonus * level;
    lines += cleared;

    // Level-up every 10 lines
    level = lines / 10 + 1;

    // Speed up (floor at 0.08 s)
    baseDelay = std::max(0.08f, 0.6f - (level - 1) * 0.05f);

    scoreText->setString("SCORE\n" + std::to_string(score));
    linesText->setString("LINES\n" + std::to_string(lines));
    levelText->setString("LEVEL\n" + std::to_string(level));
}

// ─── rotatePiece ─────────────────────────────────────────────────────────────
// BUG FIX: original undo rotation formula was wrong. Correct undo = apply
// inverse rotation (transpose sign swap reversed).
void TetrisScene::rotatePiece()
{
    TetrisPiece rotated = currentPiece;
    sf::Vector2i pivot  = rotated.blocks[1];

    for (auto& b : rotated.blocks)
    {
        int dx = b.x - pivot.x;
        int dy = b.y - pivot.y;
        b.x = pivot.x - dy;
        b.y = pivot.y + dx;
    }

    if (!checkCollision(rotated))
        currentPiece = rotated; // accept rotation
    // else: silently discard (wall kick not implemented – piece stays)
}

// ─── resetGame ───────────────────────────────────────────────────────────────
void TetrisScene::resetGame()
{
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            grid[i][j] = 0;

    score    = 0;
    lines    = 0;
    level    = 1;
    baseDelay = 0.6f;
    delay    = 0.6f;
    timer    = 0.f;
    gameOver = false;

    scoreText->setString("SCORE\n0");
    linesText->setString("LINES\n0");
    levelText->setString("LEVEL\n1");

    nextPiece.blocks.clear();
    spawnPiece();
    spawnPiece();
}

// ─── handleEvent ─────────────────────────────────────────────────────────────
void TetrisScene::handleEvent(const sf::Event& event)
{
    if (!event.is<sf::Event::KeyPressed>()) return;
    auto key = event.getIf<sf::Event::KeyPressed>()->code;

    if (key == sf::Keyboard::Key::B)
    { app->changeScene(std::make_unique<MenuScene>(app)); return; }

    if (gameOver)
    {
        if (key == sf::Keyboard::Key::R) resetGame();
        return;
    }

    if (key == sf::Keyboard::Key::Left)
    {
        for (auto& b : currentPiece.blocks) b.x--;
        if (checkCollision(currentPiece))
            for (auto& b : currentPiece.blocks) b.x++;
    }

    if (key == sf::Keyboard::Key::Right)
    {
        for (auto& b : currentPiece.blocks) b.x++;
        if (checkCollision(currentPiece))
            for (auto& b : currentPiece.blocks) b.x--;
    }

    if (key == sf::Keyboard::Key::Up)
        rotatePiece();

    // Hard drop (Space)
    if (key == sf::Keyboard::Key::Space)
    {
        while (true)
        {
            for (auto& b : currentPiece.blocks) b.y++;
            if (checkCollision(currentPiece))
            {
                for (auto& b : currentPiece.blocks) b.y--;
                lockPiece();
                break;
            }
        }
        timer = 0.f;
    }
}

// ─── update ──────────────────────────────────────────────────────────────────
// BUG FIX: original called clock.restart() every frame and added result to
// timer — but then restarted clock again at top of next frame, losing time.
// Fixed: use deltaClock continuously; restart once per frame.
void TetrisScene::update()
{
    if (gameOver) return;

    float dt = deltaClock.restart().asSeconds();
    timer += dt;

    delay = baseDelay;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        delay = 0.05f;

    if (timer >= delay)
    {
        timer = 0.f;

        for (auto& b : currentPiece.blocks) b.y++;

        if (checkCollision(currentPiece))
        {
            for (auto& b : currentPiece.blocks) b.y--;
            lockPiece();
        }
    }
}

// ─── render ──────────────────────────────────────────────────────────────────
void TetrisScene::render(sf::RenderWindow& window)
{
    auto size = app->getWindow().getSize();

    if (bgTexture.getSize().x > 0) window.draw(*bgSprite);

    sf::RectangleShape overlay({(float)size.x, (float)size.y});
    overlay.setFillColor(sf::Color(0, 0, 0, 130));
    window.draw(overlay);

    // Board background
    sf::RectangleShape board({width * tileSize, height * tileSize});
    board.setPosition({offsetX, offsetY});
    board.setFillColor(C_BOARD);
    board.setOutlineThickness(3.f);
    board.setOutlineColor(C_BORDER);
    window.draw(board);

    // Subtle grid lines
    sf::RectangleShape line;
    line.setFillColor(C_GRID);
    for (int i = 1; i < height; i++)
    {
        line.setSize({width * tileSize, 1.f});
        line.setPosition({offsetX, offsetY + i * tileSize});
        window.draw(line);
    }
    for (int j = 1; j < width; j++)
    {
        line.setSize({1.f, height * tileSize});
        line.setPosition({offsetX + j * tileSize, offsetY});
        window.draw(line);
    }

    sf::RectangleShape block({tileSize - 2.f, tileSize - 2.f});

    // Draw locked grid cells
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            if (grid[i][j])
            {
                int idx = grid[i][j] - 1;
                sf::Color col = pieceColors[idx];
                // Dark inner shade for retro look
                block.setFillColor(col);
                block.setOutlineThickness(1.f);
                block.setOutlineColor(sf::Color(
                    (std::uint8_t)(col.r / 2),
                    (std::uint8_t)(col.g / 2),
                    (std::uint8_t)(col.b / 2)
                ));
                block.setPosition({offsetX + j * tileSize + 1.f,
                                   offsetY + i * tileSize + 1.f});
                window.draw(block);
            }
    block.setOutlineThickness(0.f);

    // Draw current (falling) piece
    sf::Color curCol = pieceColors[currentPiece.type];
    block.setFillColor(curCol);
    for (auto& b : currentPiece.blocks)
    {
        if (b.y < 0) continue;
        block.setPosition({offsetX + b.x * tileSize + 1.f,
                           offsetY + b.y * tileSize + 1.f});
        window.draw(block);
    }

    // ── Sidebar ───────────────────────────────────────────────────────────────
    window.draw(*scoreText);
    window.draw(*linesText);
    window.draw(*levelText);
    window.draw(*nextLabel);
    window.draw(*infoText);

    // Next piece preview box
    float sideX  = offsetX + width * tileSize + 30.f;
    float previewY = offsetY + 290.f;

    sf::RectangleShape previewBox({6 * tileSize * 0.7f, 4 * tileSize * 0.7f});
    previewBox.setPosition({sideX, previewY});
    previewBox.setFillColor(sf::Color(10, 10, 30, 200));
    previewBox.setOutlineThickness(2.f);
    previewBox.setOutlineColor(C_BORDER);
    window.draw(previewBox);

    sf::Color nextCol = pieceColors[nextPiece.type];
    sf::RectangleShape nb({tileSize * 0.7f - 2.f, tileSize * 0.7f - 2.f});
    nb.setFillColor(nextCol);
    for (auto& b : nextPiece.blocks)
    {
        // Re-map from board coords back to 0-relative for preview
        int localX = figures[nextPiece.type][
            &b - &nextPiece.blocks[0]] % 4;
        int localY = figures[nextPiece.type][
            &b - &nextPiece.blocks[0]] / 4;
        nb.setPosition({
            sideX + 5.f + localX * tileSize * 0.7f,
            previewY + 5.f + localY * tileSize * 0.7f
        });
        window.draw(nb);
    }

    // Game-over overlay
    if (gameOver)
    {
        sf::RectangleShape dim({(float)size.x, (float)size.y});
        dim.setFillColor(sf::Color(0, 0, 0, 190));
        window.draw(dim);
        window.draw(*gameOverText);

        sf::Text sub(font, "Final Score: " + std::to_string(score) +
                           "   Lines: " + std::to_string(lines), 26);
        sub.setFillColor(C_TEXT);
        sub.setPosition({
            (float)size.x / 2.f - sub.getGlobalBounds().size.x / 2.f,
            (float)size.y / 2.f + 40.f
        });
        window.draw(sub);
    }
}
