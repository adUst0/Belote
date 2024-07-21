#include "AI.h"
#include "Utils.h"
#include "BiddingWeightGeneratorManager.h"
#include "AIConfig.h"
#include "MCTS/MCTSTree.h"
#include "BeloteGameState.h"
#include "MCTS/MCTSNode.h"
#include <unordered_map>

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
		// TODO: Refactoring - this function is too large, does tooooo many things and has some hardcoded constants

		std::vector<const Card*> possibleMoves;
		for (const Card* card : player.getCards())
		{
			if (AI::getBelote().getCurrentRound().getCurrentTrick().canPlayCard(*card, AI::getBelote().getCurrentRound().getBelote().getPlayers()[player.getPlayerIndex()].get()))
			{
				possibleMoves.push_back(card);
			}
		}
		if (possibleMoves.size() == 1)
		{
			return possibleMoves.back();
		}

		auto chooseCard = [&]() -> const Card*
		{
			PlayerHandsArray hands;
			hands[player.getPlayerIndex()].insert(hands[player.getPlayerIndex()].end(), player.getCards().begin(), player.getCards().end());

			std::vector<const Card*> hiddenCards;
			hiddenCards.reserve(player.getCards().size() * 3);
			for (const auto& player_ptr : AI::getBelote().getPlayers())
			{
				if (&player == player_ptr.get())
				{
					continue;
				}
				hiddenCards.insert(hiddenCards.end(), player_ptr->getCards().begin(), player_ptr->getCards().end());
			}

			// Randomize unknown cards between other players
			for (const auto& player_ptr : AI::getBelote().getPlayers())
			{
				if (&player == player_ptr.get())
				{
					continue;
				}

				std::vector<const Card*> cards;
				std::vector<int> weights;
				for (const Card* card : hiddenCards)
				{
					int weight = 1;

					// TODO

					cards.emplace_back(card);
					weights.emplace_back(weight);
				}

				while (hands[player_ptr->getPlayerIndex()].size() != player_ptr->getCards().size())
				{
					auto cardIter = Utils::weightedRandomSelect(cards.begin(), cards.end(), weights);
					auto weightIter = weights.begin() + std::distance(cards.begin(), cardIter);
					const Card* selectedCard = *cardIter;
					Utils::unorderedVectorErase(cards, cardIter);
					Utils::unorderedVectorErase(weights, weightIter);

					hands[player_ptr->getPlayerIndex()].push_back(selectedCard);
				}
			}

			std::unique_ptr<MCTS::MCTSGameStateBase> initialState = std::make_unique<BeloteGameState>(AI::getBelote().getCurrentRound(), hands, player.getPlayerIndex());

			const size_t maxIterations = 1000;
			MCTS::MCTSTree tree(std::move(initialState), maxIterations);
			auto node = tree.runMCTS();
			// std::cout << "Tree size is: " << tree.getTreeSize(false) << std::endl; // be careful. the tree size can be exponential

			return node ? static_cast<const BeloteGameState&>(node->getState()).getMoveToThisState().m_card : nullptr;
		};

		// Have the AI run several MCTS with different randomized hidden cards and lastly choose the move that was selected the most times
		const size_t numTries = 10;
		std::unordered_map<const Card*, size_t> moves;
		for (int i = 0; i < numTries; ++i)
		{
			moves[chooseCard()]++;
		}
		
		return std::max_element(moves.begin(), moves.end())->first;
	}

};  // namespace AI