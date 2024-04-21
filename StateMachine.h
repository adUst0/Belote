#pragma once

#include <memory>
#include <stack>

#include <SFML/Graphics.hpp>

class StateMachine;

class StateBase
{
public:
	StateBase(StateMachine& stateMachine);
	virtual ~StateBase() = default;
	StateBase(const StateBase&) = delete;
	StateBase(StateBase&&) = delete;
	StateBase& operator=(const StateBase&) = delete;
	StateBase& operator=(StateBase&&) = delete;

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

	void							pushState(std::unique_ptr<StateBase> newState, bool isReplacing = true);
	void							popState();

	void							processStateChanges();

	StateBase*						getActiveState();

private:

	std::stack<std::unique_ptr<StateBase>> m_states;
	std::unique_ptr<StateBase>			m_newState;

	bool							m_isRemoving = false;
	bool							m_isReplacing = false;
};

