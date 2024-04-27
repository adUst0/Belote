#include "UIManager.h"

void UIManager::handleInputEvent(const sf::Event& event)
{
	if (sf::Event::MouseButtonReleased == event.type && event.mouseButton.button == sf::Mouse::Button::Left)
	{
		// Start in reverse order because the last element is rendered on top
		for (auto rit = m_components.rbegin(); rit != m_components.rend(); ++rit)
		{
			if ((*rit)->handleLeftMouseClick({ (float)event.mouseButton.x, (float)event.mouseButton.y }))
			{
				break;
			}
		}
	}
	else if (sf::Event::MouseMoved == event.type)
	{
		// Start in reverse order because the last element is rendered on top
		for (auto rit = m_components.rbegin(); rit != m_components.rend(); ++rit)
		{
			if ((*rit)->handleMouseOver({ (float)event.mouseMove.x, (float)event.mouseMove.y }))
			{
				break;
			}
		}
	}
}

void UIManager::update(float dtSeconds)
{
	for (auto& component_ptr : m_components)
	{
		component_ptr->onUpdate(dtSeconds);
	}

	m_components.erase(std::remove_if(m_components.begin(), m_components.end(), [](const auto& component) { return component->isMarkedForDestruction(); }), m_components.end());
}

void UIManager::draw(sf::RenderWindow& window)
{
	for (auto& component_ptr : m_components)
	{
		window.draw(*component_ptr);
	}
}

UIComponent* UIManager::getOrCreateComponent(const std::string& key)
{
	if (UIComponent* component = getComponent(key))
	{
		return component;
	}

	m_components.emplace_back(std::make_unique<UIComponent>(key));
	return m_components.back().get();
}

UIComponent* UIManager::getComponent(const std::string& key)
{
	auto iter = std::find_if(m_components.begin(), m_components.end(), [&key](auto& ptr) { return ptr->getKey() == key; });
	return iter != m_components.end() ? (*iter).get() : nullptr;
}