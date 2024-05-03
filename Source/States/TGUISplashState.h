#pragma once

#include "StateMachine.h"
#include <TGUI/Backend/SFML-Graphics.hpp>

class TGUISplashState : public BaseState
{
public:
	TGUISplashState(StateMachine& stateMachine);

	virtual void					handleInput() override;
	virtual void					draw() override;

private:
	tgui::Gui						m_gui;
};

