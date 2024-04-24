#pragma once

#include "StateMachine.h"
#include <unordered_map>
#include "Card.h"
#include "Belote.h"
#include "Observer.h"
#include "ObserverMessages.h"

class UIComponent;

class GameState
	: public BaseState
	, public Observer<NotifyCardDealing>
	, public Observer<NotifyContractVote>
	, public Observer<NotifyCardAboutToBePlayed>
{
public:
	GameState(StateMachine& stateMachine);

	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

	virtual void					notify(const NotifyCardDealing& data) override;
	virtual void					notify(const NotifyContractVote& data) override;
	virtual void					notify(const NotifyCardAboutToBePlayed& data) override;

	void							delayGame(float seconds);
	void							togglePause();

private:

	/*
	* UIComponent
	*	setPosition - m_position
	*	setVisible - m_visible
	*	animation
	*	draw() override
	
	* UITextComponent
	* 
	*/

	struct SpriteMoveData
	{
		sf::Vector2f m_startPosition;
		sf::Vector2f m_endPosition;
		float m_elapsedTimeSeconds = 0.f;
		float m_moveTime = 0.5f;
		sf::Sprite* m_sprite = nullptr;
	};

	struct TextData
	{
		sf::Text m_text;
		float m_timeLeft = -1.f;
		bool m_deleteOnExpiration = false;
	};

	void											createCardSprites();
	sf::Vector2f									calculateCardPosition(const Player& player, int cardOrder) const;
	sf::Vector2f									getCardPositionOnTable(const Player& player) const;

	std::string										getPlayerName(const Player& player) const;

	Belote											m_belote;

	sf::Sprite										m_cardBack;

	sf::RectangleShape								m_cardBackground;
	std::unordered_map<const Card*, sf::Sprite>		m_cardSprites;
	std::unordered_map<std::string, std::unique_ptr<TextData>>	m_texts;

	std::unordered_map<std::string, std::unique_ptr<UIComponent>> m_uiComponents;

	std::unordered_map<const Card*, SpriteMoveData> m_movingSprites;

	

	sf::Sprite										m_background;

	float											m_delayGameSeconds = 0.f;
	bool											m_shouldPauseGame = false;
};

