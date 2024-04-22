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
		m_belote->returnCard(*m_cards.back());
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