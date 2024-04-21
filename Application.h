#pragma once

#include <SFML/Graphics.hpp>

#include "StateMachine.h"
#include "AssetManager.h"
#include "InputManager.h"

class Application
{
public:
	Application(unsigned int width, unsigned int height, const std::string& title);
	static Application*				getInstance() { return s_instance; }

	sf::RenderWindow&				getWindow() { return m_window; }

	StateMachine&					getStateMachine() { return m_stateMachine; }
	AssetManager&					getAssetsManager() { return m_assetManager; }
	InputManager&					getInputManager() { return m_inputManager; }

private:
	void run();

	sf::RenderWindow				m_window;

	StateMachine					m_stateMachine;
	AssetManager					m_assetManager;
	InputManager					m_inputManager;

	static Application*				s_instance;
};

