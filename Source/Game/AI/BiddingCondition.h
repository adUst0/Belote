#pragma once
#include <vector>
#include <memory>
#include "Card.h"
#include "AI.h"

namespace BiddingConditions
{
	class BiddingConditionBase
	{
	public:
		BiddingConditionBase(bool negate = false);
		virtual ~BiddingConditionBase() = default;

		bool							operator()() const;

	private:
		virtual bool					checkCondition() const = 0;

		bool							m_negate = false;
	};

	using BasePtr = std::shared_ptr<BiddingConditionBase>;

	class HaveCards : public BiddingConditionBase
	{
	public:
		HaveCards(const std::vector<Card>& cards, bool negate = false);

		static BasePtr					create(const std::vector<Card>& cards, bool negate = false);

	private:
		bool							checkCondition() const override;

		std::vector<Card>				m_cards;
	};

	class HaveAtLeastNCardsFromSuit : public BiddingConditionBase
	{
	public:
		HaveAtLeastNCardsFromSuit(Suit suit, size_t numCards, bool negate = false);

		static BasePtr					create(Suit suit, size_t numCards, bool negate = false);

	private:
		bool							checkCondition() const override;

		Suit							m_suit;
		size_t							m_numCards;
	};
}

