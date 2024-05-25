#include "AI.h"
#include "Utils.h"
#include "BiddingWeightGeneratorManager.h"
#include "AIConfig.h"

namespace
{
	size_t MAX_ITERATIONS = 100;
}

namespace AI
{
	Contract DummyAI::chooseContractVote(const Player& player)
	{
		Contract contract(Contract::Type::Pass, &player);
		size_t iterations = 0;
		do
		{
			auto contractType = Contract::Type(Utils::randRanged(0, (int8_t)Contract::Type::AllTrumps));
			contract = { contractType , &player };

		} while (!player.getBelote()->getCurrentRound().getBiddingManager().canBid(contract) && iterations++ < MAX_ITERATIONS);

		return contract;
	}

	const Card* DummyAI::chooseCardToPlay(const Player& player)
	{
		if (player.getCards().empty())
		{
			return nullptr;
		}

		for (const Card* card : player.getCards())
		{
			if (player.getBelote()->getCurrentRound().getCurrentTrick().canPlayCard(*card))
			{
				return card;
			}
		}

		return player.getCards()[0];
	}

	Contract AI::chooseContractVote(const Player& player)
	{
		static std::map<Contract::Type, BiddingWeightGeneratorManager> weightGeneratorManagers = AIConfig::createBiddingWeightGeneratorManagers();

		std::vector<std::pair<Contract, float>> contractsWeights;

		for (auto& [contractType, contractManager] : weightGeneratorManagers)
		{
			Contract contract(contractType, &player);
			if (AI::getBelote().getCurrentRound().getBiddingManager().canBid(contract))
			{
				contractsWeights.emplace_back(std::move(contract), contractManager.calculateWeight());
			}

			// TODO: doubles/redoubles
		}

		if (!contractsWeights.empty())
		{
			std::sort(contractsWeights.begin(), contractsWeights.end(), [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
			return contractsWeights.back().first;
		}

		return Contract(Contract::Type::Pass, &player);
	}

	const Card* AI::chooseCardToPlay(const Player& /*player*/)
	{
		// AllTrumps

		// Card score:
		// is 100% win pts
		// is unneeded card with many pts
		return nullptr;
	}

};  // namespace AI