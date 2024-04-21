#pragma once

#include <SFML/Graphics.hpp>

class InputManager
{
public:
	bool							isSpriteClicked(const sf::Sprite& object, sf::Mouse::Button button, const sf::RenderWindow& window) const;

	sf::Vector2i					getMousePosition(const sf::RenderWindow& window) const;
	sf::Vector2f					getMousePositionF(const sf::RenderWindow& window) const;
};
