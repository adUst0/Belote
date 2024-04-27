#pragma once
#include "UIComponent.h"
#include <vector>
#include <memory>
#include "SFML/Window/Event.hpp"

class UIManager
{
public:
	UIManager() = default;

	void					handleInputEvent(const sf::Event& event);
	void					update(float dtSeconds);
	void					draw(sf::RenderWindow& window);

	UIComponent*			getOrCreateComponent(const std::string& key);
	UIComponent*			getComponent(const std::string& key);

protected:
	std::vector<std::unique_ptr<UIComponent>>	m_components;
};