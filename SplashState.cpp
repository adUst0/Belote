#include "SplashState.h"
#include "Application.h"
#include "GameState.h"

SplashState::SplashState(StateMachine& stateMachine)
	: UIState(stateMachine)
{
	getOrCreateComponent("background")->addSprite("assets/background.jpg");

	UIComponent* startButton = getOrCreateComponent("start_button");
	startButton->setText("New Game", sf::Color::Black, 72);
	startButton->setOriginCenter(true);
	startButton->setBackground(sf::Color(255, 0, 0, 255));
	startButton->setPosition((sf::Vector2f)Application::getInstance()->getWindow().getSize() / 2.f);
	startButton->onMouseLeftClick([this]() { m_stateMachine.pushState(std::make_unique<GameState>(m_stateMachine)); });
}