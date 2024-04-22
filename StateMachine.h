#pragma once

#include <memory>
#include <stack>

#include <SFML/Graphics.hpp>

class StateMachine;

class BaseState
{
public:
	BaseState(StateMachine& stateMachine);
	virtual ~BaseState() = default;
	BaseState(const BaseState&) = delete;
	BaseState(BaseState&&) = delete;
	BaseState& operator=(const BaseState&) = delete;
	BaseState& operator=(BaseState&&) = delete;

	virtual void					onEnterState(bool /*isResume*/) {}
	virtual void					onLeaveState(bool /*isPause*/) {}

	virtual void					handleInput();
	virtual void					update(float /*dtSeconds*/) {}
	virtual void					draw() {};

protected:
	StateMachine&					m_stateMachine;
};

class StateMachine
{
public:
	StateMachine() = default;

	void							pushState(std::unique_ptr<BaseState> newState, bool isReplacing = true);
	void							popState();

	void							processStateChanges();

	BaseState*						getActiveState();

private:

	std::stack<std::unique_ptr<BaseState>> m_states;
	std::unique_ptr<BaseState>			m_newState;

	bool							m_isRemoving = false;
	bool							m_isReplacing = false;
};

