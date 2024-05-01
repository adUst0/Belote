#pragma once
#include <vector>

class Card;
class Player;
class Round;

struct TrickTurn
{
	const Card*						m_card = nullptr;
	const Player*					m_player = nullptr;
};

class Trick
{
public:
	Trick(Round* round) : m_round(round) {}

	bool							canPlayCard(const Card& card) const;
	void							playCard(const Card& card);

	const TrickTurn*				getWinningCardTurn() const;
	const Card*						getWinningCard() const;

	bool							isOver() const { return m_turns.size() == 4; }

	std::vector<const Card*>		getCards() const;

private:
	Round*							m_round = nullptr;

	std::vector<TrickTurn>			m_turns;
};

