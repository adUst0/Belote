#include "AI.h"
#include "Utils.h"

Contract DummyAI::chooseContractVote(const Player& player)
{
	Contract contract;
	do
	{
		contract = Contract(Utils::randRanged(0, (int8_t)Contract::AllTrumps));
	} while (!player.getBelote()->isValidContractVote(contract));

	return contract;
}

const Card* DummyAI::chooseCardToPlay(const Player& player)
{
	if (player.getCards().empty())
	{
		return nullptr;
	}

	const Card* card = nullptr;
	do
	{
		card = player.getCards()[Utils::randRanged(0, player.getCards().size() - 1)];
	} while (!player.getBelote()->isValidCardToPlay(*card));
	return card;
}
