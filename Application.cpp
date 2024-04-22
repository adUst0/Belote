#include "Application.h"
#include <string>
#include "States.h"
#include <cassert>

Application* Application::s_instance = nullptr;

Application::Application(unsigned int width, unsigned int height, const std::string& title)
{
	assert(!s_instance);
	s_instance = this;

	m_window.create(sf::VideoMode(width, height), title);
	m_window.setFramerateLimit(60u);

	m_stateMachine.pushState(std::make_unique<GameState>(m_stateMachine));

	run();
}

void Application::run()
{
	sf::Clock clock;

	while (m_window.isOpen())
	{
		m_stateMachine.processStateChanges();
		m_stateMachine.getActiveState()->handleInput();
		sf::Time dt = clock.restart();
		m_stateMachine.getActiveState()->update(dt.asSeconds());
		clock.restart();
		m_stateMachine.getActiveState()->draw();
	}
}