#pragma once
#include "StateMachine.h"
#include <unordered_map>
#include "Card.h"
#include "Belote.h"

using Deck = std::vector<std::unique_ptr<Card>>;

class TestRenderCardsState : public StateBase
{
public:
	TestRenderCardsState(StateMachine& stateMachine) : StateBase(stateMachine) {}

	virtual void					onEnterState(bool isResume) override;

	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

private:
	struct SpriteMoveData
	{
		sf::Vector2f m_startPosition;
		sf::Vector2f m_endPosition;
		float m_elapsedTimeSeconds = 0.f; 
		sf::Sprite* m_sprite = nullptr;
	};

	void							createDeck();

	sf::Vector2f					getCardSize() const;

	Deck							m_deck;

	sf::RectangleShape				m_cardBackground;
	std::unordered_map<const Card*, sf::Sprite> m_sprites;

	std::unordered_map<const Card*, SpriteMoveData> m_movingSprites;

	///
	Belote m_belote;

	static inline const sf::Vector2f s_cardScale{ 0.5f, 0.5f };
};

class TestBeloteState : public StateBase
{
public:
	TestBeloteState(StateMachine& stateMachine) : StateBase(stateMachine) {}

	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

private:
	Belote m_belote;

	sf::RectangleShape				m_cardBackground;

	static inline const sf::Vector2f s_cardScale{ 0.5f, 0.5f };
};