#include "InputManager.h"

sf::Vector2i InputManager::getMousePosition(const sf::RenderWindow& window) const
{
	return sf::Mouse::getPosition(window);
}

sf::Vector2f InputManager::getMousePositionF(const sf::RenderWindow& window) const
{
	return sf::Vector2f{ static_cast<float>(sf::Mouse::getPosition(window).x), static_cast<float>(sf::Mouse::getPosition(window).y) };
}
