#pragma once
#include <vector>
#include <memory>
#include "BiddingWeightGenerator.h"

namespace AI
{
	using Generators = std::vector<std::shared_ptr<BiddingWeightGenerator::BiddingWeightGeneratorBase>>;

	class BiddingWeightGeneratorManager
	{
	public:
		BiddingWeightGeneratorManager() = default;
		BiddingWeightGeneratorManager(const Generators& additiveGenerators, const Generators& multiplicativeGenerators)
			: m_additiveGenerators(additiveGenerators)
			, m_multiplicativeGenerators(multiplicativeGenerators)
		{

		}

		float calculateWeight() const
		{
			float weight = 0;

			for (auto& generator : m_additiveGenerators)
			{
				weight += generator->generateWeight();
			}

			float multiplier = 1.f;
			for (auto& generator : m_multiplicativeGenerators)
			{
				multiplier += generator->generateWeight();
			}

			weight *= multiplier;

			return weight;
		}

	private:
		Generators m_additiveGenerators;
		Generators m_multiplicativeGenerators;
	};
}
