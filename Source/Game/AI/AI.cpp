#include "AI.h"
#include "Utils.h"
#include "BiddingWeightGeneratorManager.h"
#include "AIConfig.h"
#include "MCTS/MCTSTree.h"
#include "BeloteGameState.h"
#include "MCTS/MCTSNode.h"

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

	const Card* AI::chooseCardToPlay(const Player& player)
	{
		PlayerHandsArray hands;

		// TODO: Randomize hidden cards. Implement some weighted random based on Contract voting and current play trough
		size_t i = 0;
		for (const auto& player_ptr : AI::getBelote().getPlayers())
		{
			for (const Card* card : player_ptr->getCards())
			{
				hands[i].push_back(card);
			}

			++i;
		}

		std::unique_ptr<MCTS::MCTSGameStateBase> initialState = std::make_unique<BeloteGameState>(AI::getBelote().getCurrentRound(), hands, player.getPlayerIndex());

		const size_t maxIterations = 10000;
		MCTS::MCTSTree tree(std::move(initialState), maxIterations);
		auto node = tree.runMCTS();
		// std::cout << "Tree size is: " << tree.getTreeSize(false) << std::endl;
		if (node)
		{
			return static_cast<const BeloteGameState&>(node->getState()).getMoveToThisState().m_card;
		}

		return nullptr;
	}

};  // namespace AI