#include "AI.h"
#include "Utils.h"

Contract DummyAI::chooseContractVote(const Player& player)
{
	Contract contract;
	do
	{
		contract = Contract(Utils::randRanged(0, (int8_t)Contract::Num - 1));
	} while (!player.getBelote()->isValidContractVote(contract));

	return contract;
}