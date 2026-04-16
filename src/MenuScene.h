#pragma once
#include <SFML/Graphics.hpp>
#include "Scene.h"
#include <memory>

class App;

class MenuScene : public Scene {
private:
    App* app;

    sf::Texture bgTexture;
    std::unique_ptr<sf::Sprite> bgSprite;

    sf::Font font;

    std::unique_ptr<sf::Text> titleText;
    std::unique_ptr<sf::Text> subText;
    std::unique_ptr<sf::Text> hintText;

    // Card labels + subtitles
    std::unique_ptr<sf::Text> snakeText;
    std::unique_ptr<sf::Text> snakeSub;
    std::unique_ptr<sf::Text> tetrisText;
    std::unique_ptr<sf::Text> tetrisSub;
    std::unique_ptr<sf::Text> pickleText;
    std::unique_ptr<sf::Text> pickleSub;

    // Click boxes
    sf::RectangleShape snakeBox;
    sf::RectangleShape tetrisBox;
    sf::RectangleShape pickleBox;

    // Animation
    sf::Clock animClock;
    float animTime = 0.f;

public:
    MenuScene(App* app);
    void handleEvent(const sf::Event& event) override;
    void update() override;
    void render(sf::RenderWindow& window) override;
};
