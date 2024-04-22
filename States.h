#pragma once
#include "StateMachine.h"
#include <unordered_map>
#include "Card.h"
#include "Belote.h"

using Deck = std::vector<std::unique_ptr<Card>>;

class TestRenderCardsState : public BaseState
{
public:
	TestRenderCardsState(StateMachine& stateMachine) : BaseState(stateMachine) {}

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

class TestBeloteState : public BaseState
{
public:
	TestBeloteState(StateMachine& stateMachine) : BaseState(stateMachine) {}

	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

private:
	Belote m_belote;

	sf::RectangleShape				m_cardBackground;

	std::vector<std::pair<std::unique_ptr<sf::Text>, float>> m_text;

	static inline const sf::Vector2f s_cardScale{ 0.5f, 0.5f };
};

class GameState : public BaseState
{
public:
	GameState(StateMachine& stateMachine);

	virtual void					handleInput() override;
	virtual void					update(float dtSeconds) override;
	virtual void					draw() override;

	void							notifyCardDealing(const Player& player, const Card& card);

private:
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
	};

	struct PlayerPosition
	{
		sf::Vector2f m_position;
		bool m_isHorizontallyCentered = false;
		bool m_isReversedDirection = false;
	};

	void											createCardSprites();
	sf::Vector2f									calculateCardPosition(const Player& player, int cardOrder) const;

	Belote											m_belote;

	sf::Vector2f									m_cardSize;

	sf::RectangleShape								m_cardBackground;
	std::unordered_map<const Card*, sf::Sprite>		m_cardSprites;
	std::vector<std::unique_ptr<TextData>>			m_texts;

	std::unordered_map<const Card*, SpriteMoveData> m_movingSprites;

	sf::Vector2f									m_deckPosition;
	std::vector<PlayerPosition>						m_playerPositions;

	static inline const sf::Vector2f				s_cardScale{ 0.5f, 0.5f };
	static inline const float						s_borderOffset = 20.f;
};