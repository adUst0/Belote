#include "AI.h"
#include "Utils.h"

namespace
{
	size_t MAX_ITERATIONS = 100;
}

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
