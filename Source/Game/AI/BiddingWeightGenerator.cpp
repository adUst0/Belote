#include "BiddingWeightGenerator.h"

namespace BiddingWeightGenerator
{

	BiddingWeightGeneratorBase::BiddingWeightGeneratorBase(const Preconditions& preconditions)
		: m_preconditions(preconditions)
	{}

	float BiddingWeightGeneratorBase::generateWeight() const
	{
		for (auto& condition : m_preconditions)
		{
			if (!(*condition)())
			{
				return 0.f;
			}
		}

		return generateWeightImpl();
	}

	Constant::Constant(const Preconditions& preconditions, float value)
		: BiddingWeightGeneratorBase(preconditions)
		, m_value(value)
	{}

	std::shared_ptr<BiddingWeightGenerator::BiddingWeightGeneratorBase> Constant::create(const Preconditions& preconditions, float value)
	{
		std::shared_ptr<BiddingWeightGeneratorBase> ptr = std::make_shared<Constant>(preconditions, value);
		return ptr;
	}

	WeightPerRanksFromEachSuit::WeightPerRanksFromEachSuit(
		const Preconditions& preconditions, 
		const std::vector<Rank>& requiredRanks,
		float pointsPerEachMatchingSuit,
		const std::vector<Suit>& allowedSuits)
		: BiddingWeightGeneratorBase(preconditions)
		, m_requiredRanks(requiredRanks)
		, m_pointsPerEachMatchingSuit(pointsPerEachMatchingSuit)
		, m_allowedSuits(allowedSuits)
	{
	}

	std::shared_ptr<BiddingWeightGenerator::BiddingWeightGeneratorBase> WeightPerRanksFromEachSuit::create(
		const Preconditions& preconditions, 
		const std::vector<Rank>& requiredRanks, 
		float pointsPerEachMatchingSuit,
		const std::vector<Suit>& allowedSuits /*= { Suit::Clubs, Suit::Diamonds, Suit::Hearts, Suit::Spades }*/)
	{
		std::shared_ptr<BiddingWeightGeneratorBase> ptr = std::make_shared<WeightPerRanksFromEachSuit>(preconditions, requiredRanks, pointsPerEachMatchingSuit, allowedSuits);
		return ptr;
	}

	float WeightPerRanksFromEachSuit::generateWeightImpl() const
	{
		float weight = 0.f;

		const auto& activePlayerCards = AI::AI::getBelote().getActivePlayer().getCards();

		for (Suit suit : m_allowedSuits)
		{
			const bool hasAllRanksFromSuit = std::all_of(m_requiredRanks.begin(), m_requiredRanks.end(), [&activePlayerCards, suit](Rank rank)
			{
				Card temp(suit, rank);
				return std::find_if(activePlayerCards.begin(), activePlayerCards.end(), [&temp](const Card* card) { return *card == temp; }) != activePlayerCards.end();
			});

			if (hasAllRanksFromSuit)
			{
				weight += m_pointsPerEachMatchingSuit;
			}
		}

		return weight;
	}

	WeightPerEachCardFromSuit::WeightPerEachCardFromSuit(const Preconditions& preconditions, Suit suit, float pointsPerEachCardFromSuit)
		: BiddingWeightGeneratorBase(preconditions)
		, m_suit(suit)
		, m_pointsPerEachCardFromSuit(pointsPerEachCardFromSuit)
	{

	}

	std::shared_ptr<BiddingWeightGenerator::BiddingWeightGeneratorBase> WeightPerEachCardFromSuit::create(const Preconditions& preconditions, Suit suit, float pointsPerEachCardFromSuit)
	{
		std::shared_ptr<BiddingWeightGeneratorBase> ptr = std::make_shared<WeightPerEachCardFromSuit>(preconditions, suit, pointsPerEachCardFromSuit);
		return ptr;
	}

	float WeightPerEachCardFromSuit::generateWeightImpl() const
	{
		const auto& activePlayerCards = AI::AI::getBelote().getActivePlayer().getCards();

		const size_t numCards = std::count_if(activePlayerCards.begin(), activePlayerCards.end(), [suit = m_suit](const Card* card)
		{
			return card->getSuit() == suit;
		});

		return numCards * m_pointsPerEachCardFromSuit;
	}

}