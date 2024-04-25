#include "SplashState.h"
#include "Application.h"
#include "GameState.h"

void SplashState::onEnterState(bool isResume)
{
	if (isResume)
	{
		return;
	}

	UIComponent* background = getOrCreateComponent("background");
	background->addSprite("assets/background.jpg");

	UIComponent* startButton = getOrCreateComponent("start_button");
	startButton->setText("New Game", sf::Color::Black, 72, true);
	startButton->setBackground(sf::Color(255, 0, 0, 255));
	startButton->onMouseLeftClick([this]() { m_startGame = true; });

	const sf::Vector2u size = Application::getInstance()->getWindow().getSize();
	startButton->setPosition({ size.x / 2.f, size.y / 2.f });
}

void SplashState::handleInput()
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
			for (auto rit = m_ui.rbegin(); rit != m_ui.rend(); ++rit)
			{
				if ((*rit)->handleLeftMouseClick({ (float)event.mouseButton.x, (float)event.mouseButton.y }))
				{
					break;
				}
			}
		}
	}
}

void SplashState::update(float dtSeconds)
{
	for (auto& component_ptr : m_ui)
	{
		component_ptr->onUpdate(dtSeconds);
	}

	if (m_startGame)
	{
		m_stateMachine.pushState(std::make_unique<GameState>(m_stateMachine));
	}
}

void SplashState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::White);

	for (auto& component_ptr : m_ui)
	{
		window.draw(*component_ptr);
	}

	window.display();
}

UIComponent* SplashState::getOrCreateComponent(const std::string& key)
{
	auto iter = std::find_if(m_ui.begin(), m_ui.end(), [&key](auto& ptr) { return ptr->getKey() == key; });
	if (iter != m_ui.end())
	{
		return (*iter).get();
	}

	std::unique_ptr<UIComponent>& component = m_ui.emplace_back(std::make_unique<UIComponent>(key));
	return component.get();
}
