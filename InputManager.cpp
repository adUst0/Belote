#include "InputManager.h"

bool InputManager::isSpriteClicked(const sf::Sprite& object, sf::Mouse::Button button, const sf::RenderWindow& window) const
{
	if (sf::Mouse::isButtonPressed(button))
	{
		sf::FloatRect rect(object.getPosition().x, object.getPosition().y, object.getGlobalBounds().width, object.getGlobalBounds().height);

		return rect.contains(getMousePositionF(window));
	}

	return false;
}

sf::Vector2i InputManager::getMousePosition(const sf::RenderWindow& window) const
{
	return sf::Mouse::getPosition(window);
}

sf::Vector2f InputManager::getMousePositionF(const sf::RenderWindow& window) const
{
	return sf::Vector2f{ static_cast<float>(sf::Mouse::getPosition(window).x), static_cast<float>(sf::Mouse::getPosition(window).y) };
}
