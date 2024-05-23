#include "BiddingWeightGenerator.h"

namespace BiddingWeightGenerator
{

	BiddingWeightGeneratorBase::BiddingWeightGeneratorBase(const PreconditionsVec& preconditions)
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

	Constant::Constant(const PreconditionsVec& preconditions, float value)
		: BiddingWeightGeneratorBase(preconditions)
		, m_value(value)
	{}

	std::shared_ptr<BiddingWeightGenerator::BiddingWeightGeneratorBase> Constant::create(const PreconditionsVec& preconditions, float value)
	{
		std::shared_ptr<BiddingWeightGeneratorBase> ptr = std::make_shared<Constant>(preconditions, value);
		return ptr;
	}

}