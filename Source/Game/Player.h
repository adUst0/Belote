#pragma once
#include "Card.h"
#include <vector>

class Belote;

class Player
{
public:
	Player(int teamIndex, int playerIndex, Belote& belote);

	void										addCard(const Card& card) { m_cards.push_back(&card); }

	const std::vector<const Card*>&				getCards() const { return m_cards; }
	void										returnCards();

	bool										hasSuit(Suit suit) const;

	int											getTeamIndex() const { return m_teamIndex; }
	int											getPlayerIndex() const { return m_playerIndex; }

	bool										isContractVoteRequired() const { return m_contractVoteRequired; }
	void										setContractVoteRequired();

	bool										isPlayCardRequired() const { return m_playCardRequired; }
	void										setPlayCardRequired();

	bool										isHuman() const { return m_isHuman; }

	const Belote*								getBelote() const { return m_belote; }

private:
	Belote* m_belote = nullptr;

	bool										m_isHuman = false;

	int											m_teamIndex = 0;
	int											m_playerIndex = 0;

	bool										m_contractVoteRequired = false;
	bool										m_playCardRequired = false;

	std::vector<const Card*>					m_cards;
};

