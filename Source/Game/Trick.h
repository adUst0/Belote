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
	Trick(Round* round, bool isSimulation) : m_round(round), m_isSimulation(isSimulation) {}

	bool							canPlayCard(const Card& card, const Player* activePlayer = nullptr) const;
	void							playCard(const Card& card, const Player* activePlayer = nullptr);

	bool							anyCardPlayed() const { return !m_turns.empty(); }
	const TrickTurn*				getWinningCardTurn() const;
	const Card*						getWinningCard() const;

	bool							isOver() const { return m_turns.size() == 4; }

	std::vector<const Card*>		getCards() const;

	void							setIsSimulation(bool value) { m_isSimulation = value; }
	void							setRound(Round& round) { m_round = &round; }

private:
	Round*							m_round = nullptr;

	std::vector<TrickTurn>			m_turns;

	bool							m_isSimulation = false;
};

