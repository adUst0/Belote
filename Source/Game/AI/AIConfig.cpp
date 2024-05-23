#include "AIConfig.h"

namespace AI::AIConfig
{
	std::map<Contract::Type, BiddingWeightGeneratorManager> createBiddingWeightGeneratorManagers()
	{
		std::map<Contract::Type, BiddingWeightGeneratorManager> contractWeightManagers;
	
		using namespace BiddingConditions;
		using namespace BiddingWeightGenerator;
	
		contractWeightManagers[Contract::Type::Pass] = BiddingWeightGeneratorManager{
			{/*additive*/
				Constant::create(PreconditionsVec{}, 50.f)
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Clubs] = BiddingWeightGeneratorManager{
			{/*additive*/
				Constant::create(PreconditionsVec{ HaveCards::create({"J Clubs"_c})}, 35.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"9 Clubs"_c})}, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Diamonds"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Hearts"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Spades"_c})}, 5.f),
	
				// Additional cards from the suit
				Constant::create(PreconditionsVec{ HaveCards::create({"J Clubs"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Clubs, 3) }, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Clubs"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Clubs, 4) }, 20.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Clubs"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Clubs, 5) }, 25.f),
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Diamonds] = BiddingWeightGeneratorManager{
			{/*additive*/
				Constant::create(PreconditionsVec{ HaveCards::create({"J Diamonds"_c})}, 35.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"9 Diamonds"_c})}, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Clubs"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Hearts"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Spades"_c})}, 5.f),
	
				// Additional cards from the suit
				Constant::create(PreconditionsVec{ HaveCards::create({"J Diamonds"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Diamonds, 3) }, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Diamonds"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Diamonds, 4) }, 20.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Diamonds"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Diamonds, 5) }, 25.f),
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Hearts] = BiddingWeightGeneratorManager{
			{/*additive*/
				Constant::create(PreconditionsVec{ HaveCards::create({"J Hearts"_c})}, 35.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"9 Hearts"_c})}, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Clubs"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Diamonds"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Spades"_c})}, 5.f),
	
				// Additional cards from the suit
				Constant::create(PreconditionsVec{ HaveCards::create({"J Hearts"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Hearts, 3) }, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Hearts"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Hearts, 4) }, 20.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Hearts"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Hearts, 5) }, 25.f),
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Spades] = BiddingWeightGeneratorManager{
			{/*additive*/
				Constant::create(PreconditionsVec{ HaveCards::create({"J Spades"_c})}, 35.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"9 Spades"_c})}, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Clubs"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Diamonds"_c})}, 5.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"A Hearts"_c})}, 5.f),
	
				// Additional cards from the suit
				Constant::create(PreconditionsVec{ HaveCards::create({"J Spades"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Spades, 3) }, 15.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Spades"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Spades, 4) }, 20.f),
				Constant::create(PreconditionsVec{ HaveCards::create({"J Spades"_c}), HaveAtLeastNCardsFromSuit::create(Suit::Spades, 5) }, 25.f),
			},
			{}/*multiplicative*/
		};
	
		return contractWeightManagers;
	}
}


