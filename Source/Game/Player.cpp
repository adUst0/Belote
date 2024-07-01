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

void Player::addCard(const Card& card)
{
	// Can't use lower_bound (insertion sort) because sorting might have changed if the contract has changed.
	// We need to resort when contract changed but for ease we sort on every card. MaxCards is 8 so this will not hit performance.
	m_cards.push_back(&card);
	std::sort(m_cards.begin(), m_cards.end(), [this](const Card* lhs, const Card* rhs)
	{
		const bool is_trump = lhs->getSuit() == rhs->getSuit() && m_belote->getCurrentRound().getBiddingManager().getContract().isTrumpCard(*lhs);

		const int32_t lhs_suit = (int8_t)lhs->getSuit() * 100;
		const int32_t rhs_suit = (int8_t)rhs->getSuit() * 100;

		return lhs_suit == rhs_suit ? Contract::cardRankCompare(*lhs, *rhs, is_trump) == -1 : lhs_suit < rhs_suit;
	});
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
		//Contract vote = DummyAI::chooseContractVote(*this);
		Contract vote = AI::AI::chooseContractVote(*this);
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
		//const Card* card = AI::DummyAI::chooseCardToPlay(*this);
		const Card* card = AI::AI::chooseCardToPlay(*this);
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
