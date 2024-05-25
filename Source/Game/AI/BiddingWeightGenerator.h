#pragma once
#include <vector>
#include <memory>
#include "BiddingCondition.h"

namespace BiddingWeightGenerator
{
	using Preconditions = std::vector<std::shared_ptr<BiddingConditions::BiddingConditionBase>>;

	class BiddingWeightGeneratorBase
	{
	public:
		BiddingWeightGeneratorBase(const Preconditions& preconditions);

		virtual ~BiddingWeightGeneratorBase() = default;

		float generateWeight() const;

	protected:
		virtual float generateWeightImpl() const = 0;

		Preconditions m_preconditions;
	};

	class Constant : public BiddingWeightGeneratorBase
	{
	public:
		Constant(const Preconditions& preconditions, float value);

		static std::shared_ptr<BiddingWeightGeneratorBase> create(const Preconditions& preconditions, float value);

	private:
		float generateWeightImpl() const { return m_value; }

		float m_value;
	};

	class WeightPerRanksFromEachSuit : public BiddingWeightGeneratorBase
	{
	public:
		WeightPerRanksFromEachSuit(const Preconditions& preconditions, 
			const std::vector<Rank>& requiredRanks, 
			float pointsPerEachMatchingSuit,
			const std::vector<Suit>& allowedSuits);

		static std::shared_ptr<BiddingWeightGeneratorBase> create(
			const Preconditions& preconditions, 
			const std::vector<Rank>& requiredRanks, 
			float pointsPerEachMatchingSuit, 
			const std::vector<Suit>& allowedSuits = { Suit::Clubs, Suit::Diamonds, Suit::Hearts, Suit::Spades });

	private:
		float generateWeightImpl() const;

		std::vector<Rank> m_requiredRanks;
		std::vector<Suit> m_allowedSuits;
		float m_pointsPerEachMatchingSuit;
	};

	class WeightPerEachCardFromSuit : public BiddingWeightGeneratorBase
	{
	public:
		WeightPerEachCardFromSuit(const Preconditions& preconditions,
			Suit suit,
			float pointsPerEachCardFromSuit);

		static std::shared_ptr<BiddingWeightGeneratorBase> create(
			const Preconditions& preconditions,
			Suit suit,
			float pointsPerEachCardFromSuit);

	private:
		float generateWeightImpl() const;

		Suit m_suit;
		float m_pointsPerEachCardFromSuit;
	};
}

