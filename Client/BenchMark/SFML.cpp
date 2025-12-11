#include <SFML/Graphics.hpp>
#include <iostream>
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Ship Collision");

    sf::RectangleShape ship(sf::Vector2f(50, 50));
    ship.setFillColor(sf::Color::Green);
    ship.setPosition(375, 500);

    sf::RectangleShape obstacle(sf::Vector2f(100, 50));
    obstacle.setFillColor(sf::Color::Red);
    obstacle.setPosition(350, 300);

    float speed = 300; // pixels/sec
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        float delta = clock.restart().asSeconds();
        static sf::Clock fpsClock;
        static int frames = 0;
            frames++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            fpsClock.restart();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            ship.move(-speed * delta, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            ship.move(speed * delta, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            ship.move(0, -speed * delta);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            ship.move(0, speed * delta);

        // Collision simple AABB
        if (ship.getGlobalBounds().intersects(obstacle.getGlobalBounds())) {
            ship.setFillColor(sf::Color::Yellow);
        } else {
            ship.setFillColor(sf::Color::Green);
        }

        window.clear();
        window.draw(ship);
        window.draw(obstacle);
        window.display();
    }
    return 0;
}
