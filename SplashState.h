#pragma once
#include "StateMachine.h"
#include "UIComponent.h"
#include "UIState.h"

class SplashState : public UIState
{
public:
	SplashState(StateMachine& stateMachine);
};

