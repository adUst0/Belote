#include "Player.h"
#include "Belote.h"
#include <assert.h>
#include "AI/AI.h"
#include <format>

Player::Player(int teamIndex, int playerIndex, Belote& belote)
	: m_teamIndex(teamIndex)
	, m_playerIndex(playerIndex)
	, m_belote(&belote)
	, m_contractVote(Contract::Type::Invalid, this)
{
	updateName();
}

void Player::updateName()
{
	m_nameForUI = std::format("Player {} ({})", m_playerIndex, m_isHuman ? "human" : "AI");
}

void Player::returnCards()
{
	while (!m_cards.empty())
	{
		m_belote->returnCardToDeck(*m_cards.back());
		m_cards.pop_back();
	}
}

bool Player::hasSuit(Suit suit) const
{
	return std::any_of(
		getCards().begin(), 
		getCards().end(), 
		[suit](const Card* card) { return card->getSuit() == suit; }
	);
}

void Player::setContractVoteRequired()
{
	m_contractVoteRequired = true;
	m_contractVote = Contract(Contract::Type::Invalid, this);
}

bool Player::applyContractVoteIfReady()
{
	if (!m_isHuman)
	{
		Contract vote = DummyAI::chooseContractVote(*this);
		assert(m_belote->getCurrentRound().getBiddingManager().canBid(vote));
		m_belote->getCurrentRound().getBiddingManager().bid(std::move(vote));
		m_contractVoteRequired = false;
		return true;
	}
	
	if (m_contractVote.getType() != Contract::Type::Invalid && m_belote->getCurrentRound().getBiddingManager().canBid(m_contractVote))
	{
		m_belote->getCurrentRound().getBiddingManager().bid(m_contractVote);
		m_contractVoteRequired = false;
		return true;
	}

	return false;
}

void Player::setPlayCardRequired()
{
	m_playCardRequired = true;

	if (!m_isHuman)
	{
		const Card* card = DummyAI::chooseCardToPlay(*this);
		const bool validMove = card && m_belote->getCurrentRound().getCurrentTrick().canPlayCard(*card);
		assert(validMove);
		playCard(*card);
	}
}

void Player::playCard(const Card& card)
{
	if (m_belote->getCurrentRound().getCurrentTrick().canPlayCard(card))
	{
		m_belote->getCurrentRound().getCurrentTrick().playCard(card);
		m_cards.erase(std::find(m_cards.begin(), m_cards.end(), &card));
		m_playCardRequired = false;
	}
}
