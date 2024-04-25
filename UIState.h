#pragma once
#include "StateMachine.h"
#include "UIComponent.h"

class UIState : public BaseState
{
public:
	UIState(StateMachine& stateMachine) : BaseState(stateMachine) {}

	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

	UIComponent*					getOrCreateComponent(const std::string& key);
	UIComponent*					getComponent(const std::string& key);

protected:
	std::vector<std::unique_ptr<UIComponent>>	m_uiComponents;
};