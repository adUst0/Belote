#include "StateMachine.h"

#include <SFML/Graphics.hpp>
#include "Application.h"

StateBase::StateBase(StateMachine& stateMachine)
	: m_stateMachine(stateMachine)
{
}

void StateBase::handleInput()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();

	sf::Event event;

	while (window.pollEvent(event))
	{
		if (sf::Event::Closed == event.type)
		{
			window.close();
		}
	}
}

void StateMachine::pushState(std::unique_ptr<StateBase> newState, bool isReplacing /*= true*/)
{
	m_newState = std::move(newState);
	m_isReplacing = isReplacing;
}

void StateMachine::popState()
{
	m_isRemoving = true;
}

void StateMachine::processStateChanges()
{
	if (m_isRemoving && !m_states.empty())
	{
		m_states.top()->onLeaveState(false); // not pause
		m_states.pop();
		m_isRemoving = false;

		if (!m_states.empty())
		{
			m_states.top()->onEnterState(true); // resume
		}
	}

	if (m_newState)
	{
		if (!m_states.empty())
		{
			if (m_isReplacing)
			{
				m_states.top()->onLeaveState(false); // not pause
				m_states.pop();
			}
			else
			{
				m_states.top()->onLeaveState(true); // pause
			}
		}

		m_states.push(std::move(m_newState));
		m_states.top()->onEnterState(false); // not resume
	}
}

StateBase* StateMachine::getActiveState()
{
	return m_states.empty() ? nullptr : m_states.top().get();
}