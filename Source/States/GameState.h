#pragma once

#include "StateMachine.h"
#include <unordered_map>
#include "Card.h"
#include "Belote.h"
#include "Observer.h"
#include "ObserverMessages.h"
#include "UIManager.h"

class UIComponent;

class GameState
	: public BaseState
	, public UIManager
	, public Observer<NotifyCardDealing>
	, public Observer<NotifyContractVote>
	, public Observer<NotifyCardAboutToBePlayed>
	, public Observer<NotifyEndOfTrick>
	, public Observer<NotifyEndOfRound>
	, public Observer<NotifyNewRound>
{
public:
	GameState(StateMachine& stateMachine);

	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

	virtual void					notify(const NotifyCardDealing& data) override;
	virtual void					notify(const NotifyContractVote& data) override;
	virtual void					notify(const NotifyCardAboutToBePlayed& data) override;
	virtual void					notify(const NotifyEndOfTrick& data) override;
	virtual void					notify(const NotifyEndOfRound& data) override;
	virtual void					notify(const NotifyNewRound& data) override;

	void							delayGame(float seconds);
	void							togglePause();

private:

	void											createCardSprites();
	void											createPlayerNames();

	sf::Vector2f									calculateCardPosition(const Player& player, int cardOrder, int numCards) const;
	sf::Vector2f									getCardPositionOnTable(const Player& player) const;

	Belote											m_belote;
	
	float											m_delayGameSeconds = 0.f;
	bool											m_shouldPauseGame = false;
};

