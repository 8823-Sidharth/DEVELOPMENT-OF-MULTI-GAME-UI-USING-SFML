#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Scene.h"

class App {
private:
    sf::RenderWindow window;
    std::unique_ptr<Scene> currentScene;

public:
    App();

    void run();
    void changeScene(std::unique_ptr<Scene> newScene);

    sf::RenderWindow& getWindow();
};