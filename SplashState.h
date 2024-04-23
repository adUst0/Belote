#pragma once
#include "StateMachine.h"

class SplashState : public BaseState
{
public:
	SplashState(StateMachine& stateMachine) : BaseState(stateMachine) {}

	virtual void					onEnterState(bool isResume) override;
	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

private:
	sf::Sprite						m_background;

	sf::Text						m_startGameText;
	sf::RectangleShape				m_buttonBackground;

	float							m_buttonWidth;
	float							m_buttonHeight;

	bool							m_startGame = false;

	static inline const sf::Vector2f s_buttonSize = { 400.f, 80.f };
};

