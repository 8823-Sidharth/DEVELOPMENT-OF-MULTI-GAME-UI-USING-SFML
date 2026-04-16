#include "PickleScene.h"
#include "App.h"
#include "MenuScene.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>

// ─── Retro palette ────────────────────────────────────────────────────────────
static const sf::Color C_BG      = sf::Color(10,  10,  25);
static const sf::Color C_BOARD   = sf::Color(10,  10,  40, 220);
static const sf::Color C_BORDER  = sf::Color(0,   200, 255);
static const sf::Color C_PADDLE  = sf::Color(0,   220, 255);
static const sf::Color C_BALL    = sf::Color(255, 255, 200);
static const sf::Color C_EXTBALL = sf::Color(180, 255, 180); // extra (bonus) ball
static const sf::Color C_TEXT    = sf::Color(0,   220, 255);
static const sf::Color C_LIVE    = sf::Color(255, 80,  80);

// Power capsule colours
static const sf::Color C_POW_EXPAND  = sf::Color(0,   160, 255);
static const sf::Color C_POW_DOUBLE  = sf::Color(255, 220, 0);
static const sf::Color C_POW_SPEED   = sf::Color(255, 80,  200);
static const sf::Color C_POW_EXTBALL = sf::Color(0,   255, 120);

static const sf::Color ROW_COLORS[5] = {
    sf::Color(255, 60,  60),
    sf::Color(255, 160, 0),
    sf::Color(255, 220, 0),
    sf::Color(0,   220, 100),
    sf::Color(0,   180, 255),
};

// ─── Constructor ─────────────────────────────────────────────────────────────
PickleScene::PickleScene(App* app)
: app(app)
{
    std::srand((unsigned)std::time(nullptr));

    auto size = app->getWindow().getSize();
    offsetX = (size.x - boardWidth)  / 2.f;
    offsetY = (size.y - boardHeight) / 2.f;

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        std::cout << "Font failed\n";

    if (!bgTexture.loadFromFile("assets/images/pickle_retro.png"))
        std::cout << "BG texture failed\n";

    bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    bgSprite->setScale({
        (float)size.x / (bgTexture.getSize().x > 0 ? bgTexture.getSize().x : 1),
        (float)size.y / (bgTexture.getSize().y > 0 ? bgTexture.getSize().y : 1)
    });

    paddle.setSize({paddleBaseWidth, 14.f});
    paddle.setFillColor(C_PADDLE);
    paddle.setOutlineThickness(1.f);
    paddle.setOutlineColor(sf::Color::White);

    auto makeText = [&](const std::string& s, unsigned sz, sf::Color col)
        -> std::unique_ptr<sf::Text>
    {
        auto t = std::make_unique<sf::Text>(font, s, sz);
        t->setFillColor(col);
        return t;
    };

    scoreText      = makeText("SCORE: 0",  24, C_TEXT);
    highScoreText  = makeText("BEST: 0",   24, sf::Color(255, 220, 0));
    livesText      = makeText("LIVES: 3",  24, C_LIVE);
    gameOverText   = makeText("",          40, sf::Color(255, 60, 60));
    infoText       = makeText("SPACE=Launch  LEFT/RIGHT=Move  B=Menu", 18,
                              sf::Color(100, 100, 150));
    powerStatusText= makeText("",          18, sf::Color(200, 255, 200));

    scoreText->setPosition({offsetX, offsetY - 56.f});
    highScoreText->setPosition({offsetX + boardWidth / 2.f - 60.f, offsetY - 56.f});
    livesText->setPosition({offsetX + boardWidth - 130.f, offsetY - 56.f});
    infoText->setPosition({offsetX, offsetY + boardHeight + 8.f});
    powerStatusText->setPosition({offsetX, offsetY + boardHeight + 30.f});

    buildBricks();
    reset();
}

// ─── buildBricks ─────────────────────────────────────────────────────────────
void PickleScene::buildBricks()
{
    bricks.clear();

    float brickW  = (boardWidth - 20.f) / BRICK_COLS;
    float brickH  = 24.f;
    float padding = 4.f;
    float startX  = offsetX + 10.f;
    float startY  = offsetY + 50.f;

    for (int row = 0; row < BRICK_ROWS; ++row)
    {
        for (int col = 0; col < BRICK_COLS; ++col)
        {
            Brick b;
            b.alive = true;
            b.hits  = (row == 0) ? 2 : 1;
            b.color = ROW_COLORS[row];

            b.shape.setSize({brickW - padding, brickH - padding});
            b.shape.setPosition({
                startX + col * brickW + padding / 2.f,
                startY + row * (brickH + 2.f)
            });
            b.shape.setFillColor(b.color);
            b.shape.setOutlineThickness(1.f);
            b.shape.setOutlineColor(sf::Color(
                (std::uint8_t)(b.color.r / 2),
                (std::uint8_t)(b.color.g / 2),
                (std::uint8_t)(b.color.b / 2)
            ));
            bricks.push_back(b);
        }
    }
}

// ─── reset ───────────────────────────────────────────────────────────────────
void PickleScene::reset()
{
    paddleCurrentWidth = paddleBaseWidth;
    paddle.setSize({paddleCurrentWidth, 14.f});
    paddle.setPosition({
        offsetX + boardWidth / 2.f - paddleCurrentWidth / 2.f,
        offsetY + boardHeight - 30.f
    });

    balls.clear();
    capsules.clear();

    // Create the one original ball
    BallEntry orig;
    orig.shape.setRadius(9.f);
    orig.shape.setFillColor(C_BALL);
    orig.shape.setOutlineThickness(1.f);
    orig.shape.setOutlineColor(sf::Color(200, 200, 150));
    orig.isOriginal = true;
    orig.active     = false;
    orig.velocity   = {220.f, -ballBaseSpeed};
    orig.shape.setPosition({
        paddle.getPosition().x + paddleCurrentWidth / 2.f - 9.f,
        paddle.getPosition().y - 20.f
    });
    balls.push_back(std::move(orig));

    expandTimer      = 0.f;
    doubleScoreTimer = 0.f;
    speedBoostTimer  = 0.f;
    scoreMultiplier  = 1;
}

// ─── loseLife ────────────────────────────────────────────────────────────────
void PickleScene::loseLife()
{
    lives--;
    livesText->setString("LIVES: " + std::to_string(lives));

    if (lives <= 0)
    {
        gameOver = true;
        if (score > highScore)
        {
            highScore = score;
            highScoreText->setString("BEST: " + std::to_string(highScore));
        }
        gameOverText->setString("GAME OVER!  B=Menu  R=Retry");
        gameOverText->setFillColor(sf::Color(255, 60, 60));
        auto s = app->getWindow().getSize();
        gameOverText->setPosition({
            (float)s.x / 2.f - gameOverText->getGlobalBounds().size.x / 2.f,
            (float)s.y / 2.f - 30.f
        });
    }
    else
    {
        // Keep bricks but reset balls/paddle/capsules
        capsules.clear();
        reset();
    }
}

// ─── spawnCapsule ─────────────────────────────────────────────────────────────
void PickleScene::spawnCapsule(sf::Vector2f pos)
{
    // 40% chance any destroyed brick drops a capsule
    if ((std::rand() % 100) >= 40) return;

    PowerCapsule cap;
    cap.shape.setRadius(10.f);
    cap.shape.setPosition(pos);
    cap.velocity = {0.f, 120.f};  // falls downward
    cap.alive    = true;

    // Random type
    int r = std::rand() % 4;
    switch (r)
    {
        case 0: cap.type = PowerType::EXPAND_PADDLE; cap.shape.setFillColor(C_POW_EXPAND);  break;
        case 1: cap.type = PowerType::DOUBLE_SCORE;  cap.shape.setFillColor(C_POW_DOUBLE);  break;
        case 2: cap.type = PowerType::SPEED_BOOST;   cap.shape.setFillColor(C_POW_SPEED);   break;
        case 3: cap.type = PowerType::EXTRA_BALL;    cap.shape.setFillColor(C_POW_EXTBALL); break;
    }
    cap.shape.setOutlineThickness(2.f);
    cap.shape.setOutlineColor(sf::Color::White);
    capsules.push_back(std::move(cap));
}

// ─── applyPower ──────────────────────────────────────────────────────────────
void PickleScene::applyPower(PowerType t)
{
    switch (t)
    {
        case PowerType::EXPAND_PADDLE:
            expandTimer = 10.f;
            paddleCurrentWidth = paddleBaseWidth * 1.8f;
            updatePaddleWidth();
            break;

        case PowerType::DOUBLE_SCORE:
            doubleScoreTimer = 10.f;
            scoreMultiplier  = 2;
            break;

        case PowerType::SPEED_BOOST:
            speedBoostTimer = 8.f;
            // Boost all active balls
            for (auto& b : balls)
            {
                if (!b.active) continue;
                float spd = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y);
                float factor = 1.4f;
                b.velocity *= factor;
                // Cap at 600
                float newSpd = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y);
                if (newSpd > 600.f) b.velocity *= (600.f / newSpd);
            }
            break;

        case PowerType::EXTRA_BALL:
            spawnExtraBall();
            break;
    }
}

// ─── spawnExtraBall ──────────────────────────────────────────────────────────
void PickleScene::spawnExtraBall()
{
    BallEntry extra;
    extra.shape.setRadius(9.f);
    extra.shape.setFillColor(C_EXTBALL);
    extra.shape.setOutlineThickness(1.f);
    extra.shape.setOutlineColor(sf::Color(150, 255, 150));
    extra.isOriginal = false;
    extra.active     = true;

    // Start from paddle centre with a slightly different angle
    float angle = -70.f + (std::rand() % 41) - 20.f; // -90±20 degrees ish
    float rad   = angle * 3.14159f / 180.f;
    float spd   = ballBaseSpeed + 30.f;
    extra.velocity = { spd * std::sin(rad), -spd * std::cos(rad) };

    extra.shape.setPosition({
        paddle.getPosition().x + paddleCurrentWidth / 2.f - 9.f,
        paddle.getPosition().y - 30.f
    });
    balls.push_back(std::move(extra));
}

// ─── updatePaddleWidth ───────────────────────────────────────────────────────
void PickleScene::updatePaddleWidth()
{
    float px  = paddle.getPosition().x + paddle.getSize().x / 2.f; // keep centred
    paddle.setSize({paddleCurrentWidth, 14.f});
    paddle.setPosition({px - paddleCurrentWidth / 2.f, paddle.getPosition().y});

    // Clamp inside board
    if (paddle.getPosition().x < offsetX)
        paddle.setPosition({offsetX, paddle.getPosition().y});
    if (paddle.getPosition().x + paddleCurrentWidth > offsetX + boardWidth)
        paddle.setPosition({offsetX + boardWidth - paddleCurrentWidth, paddle.getPosition().y});
}

// ─── updatePowerTimers ────────────────────────────────────────────────────────
void PickleScene::updatePowerTimers(float dt)
{
    // Expand paddle
    if (expandTimer > 0.f)
    {
        expandTimer -= dt;
        if (expandTimer <= 0.f)
        {
            expandTimer = 0.f;
            paddleCurrentWidth = paddleBaseWidth;
            updatePaddleWidth();
        }
    }

    // Double score
    if (doubleScoreTimer > 0.f)
    {
        doubleScoreTimer -= dt;
        if (doubleScoreTimer <= 0.f)
        {
            doubleScoreTimer = 0.f;
            scoreMultiplier  = 1;
        }
    }

    // Speed boost — just let it expire (balls already boosted)
    if (speedBoostTimer > 0.f) speedBoostTimer -= dt;

    // Build status string
    std::string status;
    if (expandTimer      > 0.f) status += "[WIDE " + std::to_string((int)expandTimer)      + "s] ";
    if (doubleScoreTimer > 0.f) status += "[x2 "   + std::to_string((int)doubleScoreTimer) + "s] ";
    if (speedBoostTimer  > 0.f) status += "[FAST "  + std::to_string((int)speedBoostTimer)  + "s] ";
    int extraCount = 0;
    for (auto& b : balls) if (!b.isOriginal && b.active) extraCount++;
    if (extraCount > 0) status += "[+" + std::to_string(extraCount) + " BALL] ";
    powerStatusText->setString(status);
}

// ─── handleEvent ─────────────────────────────────────────────────────────────
void PickleScene::handleEvent(const sf::Event& event)
{
    if (!event.is<sf::Event::KeyPressed>()) return;
    auto key = event.getIf<sf::Event::KeyPressed>()->code;

    if (key == sf::Keyboard::Key::B)
    { app->changeScene(std::make_unique<MenuScene>(app)); return; }

    if (gameOver || victory)
    {
        if (key == sf::Keyboard::Key::R)
        {
            score    = 0;
            lives    = 3;
            gameOver = false;
            victory  = false;
            scoreText->setString("SCORE: 0");
            livesText->setString("LIVES: 3");
            buildBricks();
            reset();
        }
        return;
    }

    if (key == sf::Keyboard::Key::Space)
    {
        // Launch all inactive balls
        for (auto& b : balls)
            if (!b.active) b.active = true;
    }
}

// ─── update ──────────────────────────────────────────────────────────────────
void PickleScene::update()
{
    if (gameOver || victory) return;

    float dt    = deltaClock.restart().asSeconds();
    float speed = 480.f;

    updatePowerTimers(dt);

    // ── Paddle movement ───────────────────────────────────────────────────────
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        paddle.move({-speed * dt, 0.f});
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        paddle.move({ speed * dt, 0.f});

    float px = paddle.getPosition().x;
    if (px < offsetX)
        paddle.setPosition({offsetX, paddle.getPosition().y});
    if (px + paddleCurrentWidth > offsetX + boardWidth)
        paddle.setPosition({offsetX + boardWidth - paddleCurrentWidth, paddle.getPosition().y});

    // ── Falling capsules ──────────────────────────────────────────────────────
    for (auto& cap : capsules)
    {
        if (!cap.alive) continue;
        cap.shape.move(cap.velocity * dt);

        // Fell out of board
        if (cap.shape.getPosition().y > offsetY + boardHeight + 60.f)
        { cap.alive = false; continue; }

        // Caught by paddle
        if (cap.shape.getGlobalBounds().findIntersection(paddle.getGlobalBounds()).has_value())
        {
            cap.alive = false;
            applyPower(cap.type);
        }
    }
    capsules.erase(std::remove_if(capsules.begin(), capsules.end(),
        [](const PowerCapsule& c){ return !c.alive; }), capsules.end());

    // ── Ball logic ────────────────────────────────────────────────────────────
    bool originalLost = false;

    for (auto& b : balls)
    {
        if (!b.active)
        {
            // Stick to paddle
            b.shape.setPosition({
                paddle.getPosition().x + paddleCurrentWidth / 2.f - b.shape.getRadius(),
                paddle.getPosition().y - b.shape.getRadius() * 2.f - 2.f
            });
            continue;
        }

        b.shape.move(b.velocity * dt);
        auto pos = b.shape.getPosition();
        float r  = b.shape.getRadius();

        // Wall collisions
        if (pos.x <= offsetX)
        { b.velocity.x = std::abs(b.velocity.x); b.shape.setPosition({offsetX, pos.y}); }
        if (pos.x + r * 2.f >= offsetX + boardWidth)
        { b.velocity.x = -std::abs(b.velocity.x); b.shape.setPosition({offsetX + boardWidth - r * 2.f, pos.y}); }
        if (pos.y <= offsetY)
        { b.velocity.y = std::abs(b.velocity.y); b.shape.setPosition({pos.x, offsetY}); }

        // Ball lost below board
        if (pos.y > offsetY + boardHeight + 60.f)
        {
            if (b.isOriginal) originalLost = true;
            b.active = false; // mark for removal (handled below)
            continue;
        }

        // Paddle collision
        auto inter = b.shape.getGlobalBounds().findIntersection(paddle.getGlobalBounds());
        if (inter.has_value() && b.velocity.y > 0.f)
        {
            b.velocity.y = -std::abs(b.velocity.y);

            float centerP = paddle.getPosition().x + paddleCurrentWidth / 2.f;
            float centerB = b.shape.getPosition().x + r;
            float off     = (centerB - centerP) / (paddleCurrentWidth / 2.f);

            float spd = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y);
            b.velocity.x = off * spd * 1.1f;

            // Speed up slightly
            float curSpd = std::sqrt(b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y);
            if (curSpd < 500.f) b.velocity *= 1.02f;
        }

        // Brick collision
        for (auto& brick : bricks)
        {
            if (!brick.alive) continue;
            auto hit = b.shape.getGlobalBounds().findIntersection(brick.shape.getGlobalBounds());
            if (!hit.has_value()) continue;

            brick.hits--;
            if (brick.hits <= 0)
            {
                brick.alive = false;
                int pts = 10 * scoreMultiplier;
                score += pts;
                scoreText->setString("SCORE: " + std::to_string(score));
                // Chance to drop a power capsule
                spawnCapsule({
                    brick.shape.getPosition().x + brick.shape.getSize().x / 2.f - 10.f,
                    brick.shape.getPosition().y
                });
            }
            else
            {
                brick.shape.setFillColor(sf::Color(
                    (std::uint8_t)(brick.color.r / 2),
                    (std::uint8_t)(brick.color.g / 2),
                    (std::uint8_t)(brick.color.b / 2)
                ));
            }

            if (hit->size.x < hit->size.y) b.velocity.x = -b.velocity.x;
            else                            b.velocity.y = -b.velocity.y;
            break;
        }
    }

    // Remove non-original lost balls (no life penalty)
    balls.erase(std::remove_if(balls.begin(), balls.end(),
        [](const BallEntry& b){ return !b.isOriginal && !b.active; }), balls.end());

    // If original was lost, handle life loss
    if (originalLost)
    {
        // Remove all extra balls too
        balls.erase(std::remove_if(balls.begin(), balls.end(),
            [](const BallEntry& b){ return !b.isOriginal; }), balls.end());
        loseLife();
        return;
    }

    // ── Victory check ─────────────────────────────────────────────────────────
    bool anyAlive = false;
    for (auto& b : bricks) if (b.alive) { anyAlive = true; break; }

    if (!anyAlive)
    {
        if (score > highScore)
        {
            highScore = score;
            highScoreText->setString("BEST: " + std::to_string(highScore));
        }
        victory = true;
        gameOverText->setString("YOU WIN!  Score: " + std::to_string(score) + "  R=Play Again  B=Menu");
        gameOverText->setFillColor(sf::Color(0, 255, 120));
        auto s = app->getWindow().getSize();
        gameOverText->setPosition({
            (float)s.x / 2.f - gameOverText->getGlobalBounds().size.x / 2.f,
            (float)s.y / 2.f - 30.f
        });
    }
}

// ─── render ──────────────────────────────────────────────────────────────────
void PickleScene::render(sf::RenderWindow& window)
{
    auto size = app->getWindow().getSize();

    if (bgTexture.getSize().x > 0) window.draw(*bgSprite);

    sf::RectangleShape overlay({(float)size.x, (float)size.y});
    overlay.setFillColor(sf::Color(0, 0, 0, 130));
    window.draw(overlay);

    // Board
    sf::RectangleShape board({boardWidth, boardHeight});
    board.setPosition({offsetX, offsetY});
    board.setFillColor(C_BOARD);
    board.setOutlineThickness(3.f);
    board.setOutlineColor(C_BORDER);
    window.draw(board);

    // Bricks
    for (auto& b : bricks)
        if (b.alive) window.draw(b.shape);

    // Power capsules — draw with icon letter inside
    for (auto& cap : capsules)
    {
        if (!cap.alive) continue;
        window.draw(cap.shape);

        // Small letter label
        char lbl = '?';
        switch (cap.type)
        {
            case PowerType::EXPAND_PADDLE: lbl = 'W'; break;
            case PowerType::DOUBLE_SCORE:  lbl = '2'; break;
            case PowerType::SPEED_BOOST:   lbl = 'S'; break;
            case PowerType::EXTRA_BALL:    lbl = '+'; break;
        }
        sf::Text t(font, std::string(1, lbl), 13);
        t.setFillColor(sf::Color::Black);
        auto gb = cap.shape.getGlobalBounds();
        t.setPosition({gb.position.x + gb.size.x / 2.f - t.getGlobalBounds().size.x / 2.f,
                       gb.position.y + gb.size.y / 2.f - t.getGlobalBounds().size.y / 2.f - 1.f});
        window.draw(t);
    }

    // Paddle
    window.draw(paddle);

    // Balls
    for (auto& b : balls)
        window.draw(b.shape);

    // Lives indicators
    for (int i = 0; i < lives; ++i)
    {
        sf::RectangleShape heart({14.f, 14.f});
        heart.setFillColor(C_LIVE);
        heart.setPosition({offsetX + boardWidth - 20.f - i * 20.f,
                           offsetY + boardHeight + 8.f});
        window.draw(heart);
    }

    // HUD
    window.draw(*scoreText);
    window.draw(*highScoreText);
    window.draw(*livesText);
    window.draw(*infoText);
    window.draw(*powerStatusText);

    // Power capsule legend (bottom right corner)
    {
        float lx = offsetX + boardWidth - 160.f;
        float ly = offsetY + boardHeight + 8.f;
        struct { sf::Color col; std::string txt; } legend[] = {
            { C_POW_EXPAND,  "W: Wide paddle" },
            { C_POW_DOUBLE,  "2: Double score" },
            { C_POW_SPEED,   "S: Speed boost" },
            { C_POW_EXTBALL, "+: Extra ball" },
        };
        for (auto& e : legend)
        {
            sf::CircleShape dot(5.f);
            dot.setFillColor(e.col);
            dot.setPosition({lx, ly + 2.f});
            window.draw(dot);
            sf::Text lt(font, e.txt, 13);
            lt.setFillColor(sf::Color(180, 180, 200));
            lt.setPosition({lx + 14.f, ly});
            window.draw(lt);
            ly += 18.f;
        }
    }

    // Overlay for game over / victory
    if (gameOver || victory)
    {
        sf::RectangleShape dim({(float)size.x, (float)size.y});
        dim.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(dim);
        window.draw(*gameOverText);

        sf::Text sub(font, "Score: " + std::to_string(score) +
                           "   Best: " + std::to_string(highScore), 26);
        sub.setFillColor(C_TEXT);
        sub.setPosition({
            (float)size.x / 2.f - sub.getGlobalBounds().size.x / 2.f,
            (float)size.y / 2.f + 30.f
        });
        window.draw(sub);
    }
}
