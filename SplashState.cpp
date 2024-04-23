#include "SplashState.h"
#include "Application.h"
#include "GameState.h"

void SplashState::onEnterState(bool isResume)
{
	if (isResume)
	{
		return;
	}

	Application::getInstance()->getAssetsManager().loadTexture("assets/background.jpg", "assets/background.jpg");
	m_background.setTexture(Application::getInstance()->getAssetsManager().getTexture("assets/background.jpg"), true);

	m_buttonBackground.setSize(s_buttonSize);
	m_buttonBackground.setOrigin({ s_buttonSize.x / 2.f, s_buttonSize.y / 2.f });
	m_buttonBackground.setFillColor(sf::Color(255, 0, 0, 255));

	m_startGameText.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());
	m_startGameText.setString("New Game");
	m_startGameText.setCharacterSize(72);
	m_startGameText.setFillColor(sf::Color::Black);
	// Center
	m_startGameText.setOrigin({ m_startGameText.getGlobalBounds().width / 2.f + m_startGameText.getLocalBounds().left, m_startGameText.getGlobalBounds().height / 2.f + +m_startGameText.getLocalBounds().top });

	const sf::Vector2u size = Application::getInstance()->getWindow().getSize();

	m_buttonBackground.setPosition({ size.x / 2.f, size.y / 2.f });
	m_startGameText.setPosition({ size.x / 2.f, size.y / 2.f });
}

void SplashState::handleInput()
{
	BaseState::handleInput();

	if (Application::getInstance()->getInputManager().isObjectClicked(m_startGameText, sf::Mouse::Button::Left, Application::getInstance()->getWindow()))
	{
		m_startGame = true;
	}

	const bool hovered = Application::getInstance()->getInputManager().isMouseOverObject(m_buttonBackground, Application::getInstance()->getWindow());
	m_buttonBackground.setFillColor(sf::Color(255, 0, 0, hovered ? 200 : 255));
}

void SplashState::update(float dtSeconds)
{
	if (m_startGame)
	{
		m_stateMachine.pushState(std::make_unique<GameState>(m_stateMachine));
	}
}

void SplashState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::White);

	window.draw(m_background);
	window.draw(m_buttonBackground);
	window.draw(m_startGameText);

	window.display();
}
