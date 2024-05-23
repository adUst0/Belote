#include "BiddingCondition.h"

namespace BiddingConditions
{

	BiddingConditionBase::BiddingConditionBase(bool negate /*= false*/) 
		: m_negate(negate)
	{}

	bool BiddingConditionBase::operator()() const
	{
		return m_negate ? !checkCondition() : checkCondition();
	}

	HaveCards::HaveCards(const std::vector<Card>& cards, bool negate /*= false*/) 
		: BiddingConditionBase(negate), m_cards(cards)
	{}

	std::shared_ptr<BiddingConditions::BiddingConditionBase> HaveCards::create(const std::vector<Card>& cards, bool negate /*= false*/)
	{
		std::shared_ptr<BiddingConditionBase> ptr = std::make_shared<HaveCards>(cards, negate);
		return ptr;
	}

	bool HaveCards::checkCondition() const
	{
		const auto& activePlayerCards = AI::AI::getBelote().getActivePlayer().getCards();
		return std::all_of(m_cards.begin(), m_cards.end(), [&activePlayerCards](const Card& requiredCard)
		{
			return std::find_if(activePlayerCards.begin(), activePlayerCards.end(), [&requiredCard](const Card* card)
			{
				return *card == requiredCard;
			}) != activePlayerCards.end();
		});
	}

	HaveAtLeastNCardsFromSuit::HaveAtLeastNCardsFromSuit(Suit suit, size_t numCards, bool negate /*= false*/)
		: BiddingConditionBase(negate)
		, m_suit(suit)
		, m_numCards(numCards)
	{}

	std::shared_ptr<BiddingConditions::BiddingConditionBase> HaveAtLeastNCardsFromSuit::create(Suit suit, size_t numCards, bool negate /*= false*/)
	{
		std::shared_ptr<BiddingConditionBase> ptr = std::make_shared<HaveAtLeastNCardsFromSuit>(suit, numCards, negate);
		return ptr;
	}

	bool HaveAtLeastNCardsFromSuit::checkCondition() const
	{
		const auto& activePlayerCards = AI::AI::getBelote().getActivePlayer().getCards();
		return std::count_if(activePlayerCards.begin(), activePlayerCards.end(), [suit = m_suit](const Card* card)
		{
			return card->getSuit() == suit;
		}) >= (int)m_numCards;
	}

}