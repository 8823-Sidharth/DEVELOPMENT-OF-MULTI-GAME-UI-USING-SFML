#include "App.h"
#include "MenuScene.h"

App::App()
: window(
    sf::VideoMode::getDesktopMode(),
    "Gaming Platform",
    sf::Style::Default
)
{
    window.setFramerateLimit(60);

    // ✅ FIX VIEW (SFML 3)
    sf::View view;
    view.setSize(sf::Vector2f(
        (float)window.getSize().x,
        (float)window.getSize().y
    ));
    view.setCenter(view.getSize() / 2.f);
    window.setView(view);

    currentScene = std::make_unique<MenuScene>(this);
}

void App::run()
{
    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            currentScene->handleEvent(*event);
        }

        currentScene->update();

        window.clear(sf::Color(20, 20, 30));
        currentScene->render(window);
        window.display();
    }
}

void App::changeScene(std::unique_ptr<Scene> newScene)
{
    currentScene = std::move(newScene);
}

sf::RenderWindow& App::getWindow()
{
    return window;
}