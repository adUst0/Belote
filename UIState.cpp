#include "UIState.h"
#include "Application.h"

void UIState::handleInput()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();

	sf::Event event;

	while (window.pollEvent(event))
	{
		if (sf::Event::Closed == event.type)
		{
			window.close();
		}
		else if (sf::Event::MouseButtonReleased == event.type && event.mouseButton.button == sf::Mouse::Button::Left)
		{
			// Start in reverse order because the last element is rendered on top
			for (auto rit = m_uiComponents.rbegin(); rit != m_uiComponents.rend(); ++rit)
			{
				if ((*rit)->handleLeftMouseClick({ (float)event.mouseButton.x, (float)event.mouseButton.y }))
				{
					break;
				}
			}
		}
	}
}

void UIState::update(float dtSeconds)
{
	for (auto& component_ptr : m_uiComponents)
	{
		component_ptr->onUpdate(dtSeconds);
	}
}

void UIState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::White);

	for (auto& component_ptr : m_uiComponents)
	{
		window.draw(*component_ptr);
	}

	window.display();
}

UIComponent* UIState::getOrCreateComponent(const std::string& key)
{
	if (UIComponent* component = getComponent(key))
	{
		return component;
	}

	m_uiComponents.emplace_back(std::make_unique<UIComponent>(key));
	return m_uiComponents.back().get();
}

UIComponent* UIState::getComponent(const std::string& key)
{
	auto iter = std::find_if(m_uiComponents.begin(), m_uiComponents.end(), [&key](auto& ptr) { return ptr->getKey() == key; });
	return iter != m_uiComponents.end() ? (*iter).get() : nullptr;
}
