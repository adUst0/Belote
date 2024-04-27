#include "AI.h"
#include "Utils.h"

namespace
{
	size_t MAX_ITERATIONS = 100;
}

Contract DummyAI::chooseContractVote(const Player& player)
{
	Contract contract;
	size_t iterations = 0;
	do
	{
		contract = Contract(Utils::randRanged(0, (int8_t)Contract::AllTrumps));
	} while (!player.getBelote()->isValidContractVote(contract) && iterations++ < MAX_ITERATIONS);

	return contract;
}

const Card* DummyAI::chooseCardToPlay(const Player& player)
{
	if (player.getCards().empty())
	{
		return nullptr;
	}

	//const Card* card = nullptr;
	//size_t iterations = 0;
	//do
	//{
	//	card = player.getCards()[Utils::randRanged(0, player.getCards().size() - 1)];
	//} while (!player.getBelote()->isValidCardToPlay(*card) && iterations++ < MAX_ITERATIONS);
	//return card;

	for (const Card* card : player.getCards())
	{
		if (player.getBelote()->isValidCardToPlay(*card))
		{
			return card;
		}
	}

	return player.getCards()[0];
}
