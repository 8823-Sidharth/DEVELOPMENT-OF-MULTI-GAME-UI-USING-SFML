#include "MenuScene.h"
#include "App.h"
#include "SnakeScene.h"
#include "TetrisScene.h"
#include "PickleScene.h"
#include <cmath>
#include <iostream>
#include <cstdint>

// ─── Retro palette ────────────────────────────────────────────────────────────
static const sf::Color C_BG      = sf::Color(8,   8,   20);
static const sf::Color C_BORDER  = sf::Color(0,   200, 255);
static const sf::Color C_HOVER   = sf::Color(0,   255, 180);
static const sf::Color C_TITLE   = sf::Color(255, 220, 0);
static const sf::Color C_LABEL   = sf::Color(200, 200, 255);
static const sf::Color C_SUB     = sf::Color(80,  80,  130);
static const sf::Color C_SNAKE   = sf::Color(0,   255, 120);
static const sf::Color C_TETRIS  = sf::Color(0,   180, 255);
static const sf::Color C_PICKLE  = sf::Color(255, 160, 0);
// ─────────────────────────────────────────────────────────────────────────────

MenuScene::MenuScene(App* app)
: app(app)
{
    auto size = app->getWindow().getSize();

    if (!font.openFromFile("assets/fonts/ARIAL.TTF"))
        std::cout << "Font failed\n";

    if (!bgTexture.loadFromFile("assets/images/menu_bg.png"))
        std::cout << "Background failed\n";

    bgSprite = std::make_unique<sf::Sprite>(bgTexture);
    bgSprite->setScale({
        (float)size.x / (bgTexture.getSize().x > 0 ? bgTexture.getSize().x : 1),
        (float)size.y / (bgTexture.getSize().y > 0 ? bgTexture.getSize().y : 1)
    });

    // ── Title ─────────────────────────────────────────────────────────────────
    titleText = std::make_unique<sf::Text>(font, "RETRO ARCADE", 72);
    titleText->setFillColor(C_TITLE);
    titleText->setOutlineColor(sf::Color(180, 100, 0));
    titleText->setOutlineThickness(3.f);
    titleText->setPosition({
        (float)size.x / 2.f - titleText->getGlobalBounds().size.x / 2.f, 60.f });

    subText = std::make_unique<sf::Text>(font, "CHOOSE YOUR GAME", 22);
    subText->setFillColor(C_SUB);
    subText->setPosition({
        (float)size.x / 2.f - subText->getGlobalBounds().size.x / 2.f, 155.f });

    hintText = std::make_unique<sf::Text>(font, "CLICK A GAME TO PLAY   |   ESC = QUIT", 18);
    hintText->setFillColor(sf::Color(60, 60, 100));
    hintText->setPosition({
        (float)size.x / 2.f - hintText->getGlobalBounds().size.x / 2.f,
        (float)size.y - 40.f });

    // ── Card layout ───────────────────────────────────────────────────────────
    float boxW    = 220.f;
    float boxH    = 200.f;
    float spacing = 60.f;
    float total   = 3.f * boxW + 2.f * spacing;
    float startX  = ((float)size.x - total) / 2.f;
    float cardY   = (float)size.y / 2.f - boxH / 2.f + 20.f;

    // Positions
    snakeBox.setSize({boxW, boxH});
    tetrisBox.setSize({boxW, boxH});
    pickleBox.setSize({boxW, boxH});

    snakeBox.setPosition({startX,                   cardY});
    tetrisBox.setPosition({startX + boxW + spacing,  cardY});
    pickleBox.setPosition({startX + 2.f*(boxW+spacing), cardY});

    auto styleBox = [](sf::RectangleShape& box, sf::Color col) {
        box.setFillColor(sf::Color(col.r/8, col.g/8, col.b/8, 200));
        box.setOutlineThickness(2.5f);
        box.setOutlineColor(col);
    };
    styleBox(snakeBox,  C_SNAKE);
    styleBox(tetrisBox, C_TETRIS);
    styleBox(pickleBox, C_PICKLE);

    // ── Card labels ───────────────────────────────────────────────────────────
    auto makeLabel = [&](const std::string& title, const std::string& sub, sf::Color col,
                         const sf::RectangleShape& box)
        -> std::pair<std::unique_ptr<sf::Text>, std::unique_ptr<sf::Text>>
    {
        auto t = std::make_unique<sf::Text>(font, title, 34);
        t->setFillColor(col);
        auto bpos = box.getPosition();
        auto bsize = box.getSize();
        t->setPosition({
            bpos.x + bsize.x/2.f - t->getGlobalBounds().size.x/2.f,
            bpos.y + bsize.y/2.f - 30.f
        });

        auto s = std::make_unique<sf::Text>(font, sub, 16);
        s->setFillColor(sf::Color(col.r/2, col.g/2+60, col.b/2+60, 220));
        s->setPosition({
            bpos.x + bsize.x/2.f - s->getGlobalBounds().size.x/2.f,
            bpos.y + bsize.y/2.f + 14.f
        });
        return {std::move(t), std::move(s)};
    };

    auto [st, ss] = makeLabel("SNAKE",      "Eat. Grow. Survive.",    C_SNAKE,  snakeBox);
    auto [tt, ts] = makeLabel("TETRIS",     "Stack. Clear. Score.",   C_TETRIS, tetrisBox);
    auto [pt, ps] = makeLabel("BREAKOUT",   "Smash every brick!",     C_PICKLE, pickleBox);

    snakeText    = std::move(st); snakeSub    = std::move(ss);
    tetrisText   = std::move(tt); tetrisSub   = std::move(ts);
    pickleText   = std::move(pt); pickleSub   = std::move(ps);
}

void MenuScene::handleEvent(const sf::Event& event)
{
    if (event.is<sf::Event::Closed>())
        app->getWindow().close();

    if (event.is<sf::Event::KeyPressed>())
    {
        auto key = event.getIf<sf::Event::KeyPressed>()->code;
        if (key == sf::Keyboard::Key::Escape)
            app->getWindow().close();
    }

    if (!event.is<sf::Event::MouseButtonPressed>()) return;

    auto mouse = (sf::Vector2f)event.getIf<sf::Event::MouseButtonPressed>()->position;

    if (snakeBox.getGlobalBounds().contains(mouse))
        app->changeScene(std::make_unique<SnakeScene>(app));

    if (tetrisBox.getGlobalBounds().contains(mouse))
        app->changeScene(std::make_unique<TetrisScene>(app));

    if (pickleBox.getGlobalBounds().contains(mouse))
        app->changeScene(std::make_unique<PickleScene>(app));
}

void MenuScene::update()
{
    animTime += animClock.restart().asSeconds();

    // Pulsing title scale
    float pulse = 1.f + 0.015f * std::sin(animTime * 2.5f);
    titleText->setScale({pulse, pulse});

    // Blink sub text
    std::uint8_t alpha = (std::uint8_t)(160 + 95 * std::sin(animTime * 3.f));
    subText->setFillColor(sf::Color(C_SUB.r, C_SUB.g, C_SUB.b, alpha));

    // Hover detection
    auto mousePos = (sf::Vector2f)sf::Mouse::getPosition(app->getWindow());

    auto applyHover = [&](sf::RectangleShape& box, sf::Color baseCol)
    {
        if (box.getGlobalBounds().contains(mousePos))
        {
            float t = 0.5f + 0.5f * std::sin(animTime * 6.f);
            std::uint8_t a = (std::uint8_t)(160 + 60.f * t);
            box.setFillColor(sf::Color(baseCol.r/5, baseCol.g/5, baseCol.b/5, a));
            box.setOutlineColor(sf::Color(
                std::min(255, (int)(baseCol.r * 1.4f)),
                std::min(255, (int)(baseCol.g * 1.4f)),
                std::min(255, (int)(baseCol.b * 1.4f))
            ));
            box.setOutlineThickness(4.f);
        }
        else
        {
            box.setFillColor(sf::Color(baseCol.r/8, baseCol.g/8, baseCol.b/8, 200));
            box.setOutlineColor(baseCol);
            box.setOutlineThickness(2.5f);
        }
    };

    applyHover(snakeBox,  C_SNAKE);
    applyHover(tetrisBox, C_TETRIS);
    applyHover(pickleBox, C_PICKLE);
}

void MenuScene::render(sf::RenderWindow& window)
{
    auto size = window.getSize();

    // Background
    if (bgTexture.getSize().x > 0)
        window.draw(*bgSprite);
    else
    {
        window.clear(C_BG);
    }

    // Dark overlay
    sf::RectangleShape overlay({(float)size.x, (float)size.y});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    // Scanline effect (horizontal lines every 4 px)
    sf::RectangleShape scanline({(float)size.x, 1.f});
    scanline.setFillColor(sf::Color(0, 0, 0, 40));
    for (unsigned y = 0; y < size.y; y += 4)
    {
        scanline.setPosition({0.f, (float)y});
        window.draw(scanline);
    }

    // Cards
    window.draw(snakeBox);
    window.draw(tetrisBox);
    window.draw(pickleBox);

    // Decorative corner brackets on each card
    auto drawCorners = [&](const sf::RectangleShape& box, sf::Color col)
    {
        float x = box.getPosition().x, y = box.getPosition().y;
        float w = box.getSize().x,     h = box.getSize().y;
        float len = 18.f, thick = 3.f;

        sf::RectangleShape line;
        line.setFillColor(col);

        // Top-left
        line.setSize({len, thick}); line.setPosition({x, y}); window.draw(line);
        line.setSize({thick, len}); line.setPosition({x, y}); window.draw(line);
        // Top-right
        line.setSize({len, thick}); line.setPosition({x+w-len, y}); window.draw(line);
        line.setSize({thick, len}); line.setPosition({x+w-thick, y}); window.draw(line);
        // Bottom-left
        line.setSize({len, thick}); line.setPosition({x, y+h-thick}); window.draw(line);
        line.setSize({thick, len}); line.setPosition({x, y+h-len}); window.draw(line);
        // Bottom-right
        line.setSize({len, thick}); line.setPosition({x+w-len, y+h-thick}); window.draw(line);
        line.setSize({thick, len}); line.setPosition({x+w-thick, y+h-len}); window.draw(line);
    };

    drawCorners(snakeBox,  C_SNAKE);
    drawCorners(tetrisBox, C_TETRIS);
    drawCorners(pickleBox, C_PICKLE);

    // Card text
    window.draw(*snakeText);   window.draw(*snakeSub);
    window.draw(*tetrisText);  window.draw(*tetrisSub);
    window.draw(*pickleText);  window.draw(*pickleSub);

    // Title + sub
    window.draw(*titleText);
    window.draw(*subText);
    window.draw(*hintText);
}
