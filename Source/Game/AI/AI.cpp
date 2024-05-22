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

Contract AI::chooseContractVote(const Player& player)
{
	/*
	* Condition: class with operator()
		* haveCards(cards[])
		* haveAtLeastNCardsFromColor(color)
		* hasTeamVotedForOtherColorThan(whichTeam, color)
		* hasPlayerVotedAnythingDifferentThanPass(player)
		* willTeamPlayFirst(whichTeam)
	* AdditiveBonus(list of conditions, initialValue, step[[maybe unused]])
	* MultiplicativeBonus(list of conditions, value)
	* 
	* AdditiveBonuses
		* Constant(no conditions, constant value) - used for Pass
		* HaveCardsFlatBonus(preconditions list, cards[], value)
		* ProgressiveBonusForEachCardFromColour(preconditions, Colour, initialValue, step) example with Clubs: precondition is haveCards(J and 9) and haveAtLeastNCardsFromColor(3), initialValue=-5, step=5. So 3 cards is 10, 4 cards is 15, 5 cards is 20
	*/


	// Pass = 49%
	// Clubs - pts for J, 9. Points for each card. Points for declarations. Points for A from different suits. Multiplier for lower colour from Ally / Enemy
		// J=35, 9=15, each of the same color=5*(num-1), each A from other color = 5. If ally said colour & this is your first vote & your team is first *1.20. If enemy said color, *1.1
	
	// Diamonds
	// Hearts
	// Spades
	// No Trumps - pts for each A. *2 if we have 10 of same color. Bonus if no J / 9. Multiplier if current contract is colour from enemy and we don't have of it.
		// A=20, A+10=40 + 5 for each additional, J=-2, 9=-1. If ally said colour *0.6. If enemy said colour and you don't have it *1.2
	
	// All Trumps - pts for J, pts for J + 9.
		// Pts for J = 25, Pts for J+9 = 35 + 5 for each additional from colour, Pts for 9+another 2 cards = 15. | -20% for each color said by enemy | +40% for each color said by ally
 
	// Double
		// if you have the points for this vote and it is called by the enemy
	// Redouble
		// // if you have the points for this vote * 0.8 and it is called by the enemy
	return Contract(Contract::Type::Pass, &player);
}

const Card* AI::chooseCardToPlay(const Player& player)
{
	// AllTrumps

	// Card score:
	// is 100% win pts
	// is unneeded card with many pts
	return nullptr;
}
