#pragma once

#include <SFML/Graphics.hpp>

class InputManager
{
public:
	template <typename T>
	bool							isObjectClicked(const T& object, sf::Mouse::Button button, const sf::RenderWindow& window) const;

	template <typename T>
	bool							isMouseOverObject(const T& object, const sf::RenderWindow& window) const;

	sf::Vector2i					getMousePosition(const sf::RenderWindow& window) const;
	sf::Vector2f					getMousePositionF(const sf::RenderWindow& window) const;
};

template <typename T>
bool InputManager::isObjectClicked(const T& object, sf::Mouse::Button button, const sf::RenderWindow& window) const
{
	return sf::Mouse::isButtonPressed(button) && isMouseOverObject(object, window);
}

template <typename T>
bool InputManager::isMouseOverObject(const T& object, const sf::RenderWindow& window) const
{
	sf::FloatRect rect(object.getGlobalBounds().left, object.getGlobalBounds().top, object.getGlobalBounds().width, object.getGlobalBounds().height);

	return rect.contains(getMousePositionF(window));
}
