#pragma once
#include <vector>
#include <memory>
#include "BiddingCondition.h"

namespace BiddingWeightGenerator
{
	using PreconditionsVec = std::vector<std::shared_ptr<BiddingConditions::BiddingConditionBase>>;

	class BiddingWeightGeneratorBase
	{
	public:
		BiddingWeightGeneratorBase(const PreconditionsVec& preconditions);

		virtual ~BiddingWeightGeneratorBase() = default;

		float generateWeight() const;

	protected:
		virtual float generateWeightImpl() const = 0;

		PreconditionsVec m_preconditions;
	};

	class Constant : public BiddingWeightGeneratorBase
	{
	public:
		Constant(const PreconditionsVec& preconditions, float value);

		static std::shared_ptr<BiddingWeightGeneratorBase> create(const PreconditionsVec& preconditions, float value);

	private:
		float generateWeightImpl() const { return m_value; }

		float m_value;
	};
}

