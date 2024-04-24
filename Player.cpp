#include "Player.h"
#include "Belote.h"
#include <assert.h>
#include "AI.h"

Player::Player(int teamIndex, int playerIndex, Belote& belote)
	: m_teamIndex(teamIndex)
	, m_playerIndex(playerIndex)
	, m_belote(&belote)
{
}

void Player::returnCards()
{
	while (!m_cards.empty())
	{
		m_belote->returnCardToDeck(*m_cards.back());
		m_cards.pop_back();
	}
}

void Player::setContractVoteRequired()
{
	m_contractVoteRequired = true;

	if (!m_isHuman)
	{
		const Contract vote = DummyAI::chooseContractVote(*this);
		assert(m_belote->isValidContractVote(vote));
		m_belote->voteForContract(vote);
		m_contractVoteRequired = false;
	}
}

void Player::setPlayCardRequired()
{
	m_playCardRequired = true;

	if (!m_isHuman)
	{
		const Card* card = DummyAI::chooseCardToPlay(*this);
		assert(card && m_belote->isValidCardToPlay(*card));
		m_belote->playCard(*card);
		m_cards.erase(std::find(m_cards.begin(), m_cards.end(), card));
		m_playCardRequired = false;
	}
}
