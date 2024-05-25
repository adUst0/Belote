#include "AIConfig.h"

namespace
{
	static constexpr float PASS_WEIGHT								= 50.f;

	static constexpr float SUIT_TRUMP_J_WEIGHT						= 35.f;
	static constexpr float SUIT_TRUMP_9_WEIGHT						= 15.f;
	static constexpr float SUIT_NON_TRUMP_A_WEIGHT					= 5.f;
	static constexpr float SUIT_TRUMP_CARD_WEIGHT					= 5.f;
	static const std::vector<Suit> CLUBS_NON_TRUMP_SUITS			= { Suit::Diamonds, Suit::Hearts, Suit::Spades };
	static const std::vector<Suit> DIAMONDS_NON_TRUMP_SUITS			= { Suit::Clubs, Suit::Hearts, Suit::Spades };
	static const std::vector<Suit> HEARTS_NON_TRUMP_SUITS			= { Suit::Diamonds, Suit::Clubs, Suit::Spades };
	static const std::vector<Suit> SPADES_NON_TRUMP_SUITS			= { Suit::Diamonds, Suit::Hearts, Suit::Clubs };

	static constexpr float NO_TRUMPS_A_WEIGHT						= 15.f;
	static constexpr float NO_TRUMPS_A_10_WEIGHT					= 20.f;
	static constexpr float NO_TRUMPS_ADDITIONAL_CARD_WEIGHT			= 5.f;
	static constexpr float NO_TRUMPS_J_WEIGHT						= -2.f;
	static constexpr float NO_TRUMPS_9_WEIGHT						= -1.f;

	static constexpr float ALL_TRUMPS_J_WEIGHT						= 25.f;
	static constexpr float ALL_TRUMPS_J_9_WEIGHT					= 15.f;
	static constexpr float ALL_TRUMPS_ADDITIONAL_CARD_WEIGHT		= 5.f;
	static constexpr float ALL_TRUMPS_9_AND_2_OTHER_CARDS_WEIGHT	= 15.f;
}

namespace AI::AIConfig
{
	std::map<Contract::Type, BiddingWeightGeneratorManager> createBiddingWeightGeneratorManagers()
	{
		std::map<Contract::Type, BiddingWeightGeneratorManager> contractWeightManagers;
	
		using namespace BiddingConditions;
		using namespace BiddingWeightGenerator;
	
		contractWeightManagers[Contract::Type::Pass] = {
			{/*additive*/
				Constant::create({}, PASS_WEIGHT)
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Clubs] = {
			{/*additive*/
				Constant::create({ HaveCards::create({"J Clubs"_c}) }, SUIT_TRUMP_J_WEIGHT),
				Constant::create({ HaveCards::create({"9 Clubs"_c}) }, SUIT_TRUMP_9_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Ace }, SUIT_NON_TRUMP_A_WEIGHT, CLUBS_NON_TRUMP_SUITS),

				// Additional cards from the suit
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Clubs"_c}) }, Suit::Clubs, SUIT_TRUMP_CARD_WEIGHT),
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Diamonds] = {
			{/*additive*/
				Constant::create({ HaveCards::create({"J Diamonds"_c}) }, SUIT_TRUMP_J_WEIGHT),
				Constant::create({ HaveCards::create({"9 Diamonds"_c}) }, SUIT_TRUMP_9_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Ace }, SUIT_NON_TRUMP_A_WEIGHT, DIAMONDS_NON_TRUMP_SUITS),
	
				// Additional cards from the suit
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Clubs"_c}) }, Suit::Diamonds, SUIT_TRUMP_CARD_WEIGHT),
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Hearts] = {
			{/*additive*/
				Constant::create({ HaveCards::create({"J Hearts"_c}) }, SUIT_TRUMP_J_WEIGHT),
				Constant::create({ HaveCards::create({"9 Hearts"_c}) }, SUIT_TRUMP_9_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Ace }, SUIT_NON_TRUMP_A_WEIGHT, HEARTS_NON_TRUMP_SUITS),
	
				// Additional cards from the suit
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Clubs"_c}) }, Suit::Hearts, SUIT_TRUMP_CARD_WEIGHT),
			},
			{}/*multiplicative*/
		};
	
		contractWeightManagers[Contract::Type::Spades] = {
			{/*additive*/
				Constant::create({ HaveCards::create({"J Spades"_c}) }, SUIT_TRUMP_J_WEIGHT),
				Constant::create({ HaveCards::create({"9 Spades"_c}) }, SUIT_TRUMP_9_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Ace }, SUIT_NON_TRUMP_A_WEIGHT, SPADES_NON_TRUMP_SUITS),
	
				// Additional cards from the suit
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Clubs"_c}) }, Suit::Spades, SUIT_TRUMP_CARD_WEIGHT),
			},
			{}/*multiplicative*/
		};

		contractWeightManagers[Contract::Type::NoTrumps] = {
			{/*additive*/
				WeightPerRanksFromEachSuit::create({}, { Rank::Ace }, NO_TRUMPS_A_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Ace, Rank::Ten }, NO_TRUMPS_A_10_WEIGHT),

				// Additional cards from the suit
				WeightPerEachCardFromSuit::create({ HaveCards::create({"A Clubs"_c, "10 Clubs"_c}) }, Suit::Clubs, NO_TRUMPS_ADDITIONAL_CARD_WEIGHT),
				WeightPerEachCardFromSuit::create({ HaveCards::create({"A Diamonds"_c, "10 Diamonds"_c}) }, Suit::Diamonds, NO_TRUMPS_ADDITIONAL_CARD_WEIGHT),
				WeightPerEachCardFromSuit::create({ HaveCards::create({"A Hearts"_c, "10 Hearts"_c}) }, Suit::Hearts, NO_TRUMPS_ADDITIONAL_CARD_WEIGHT),
				WeightPerEachCardFromSuit::create({ HaveCards::create({"A Spades"_c, "10 Spades"_c}) }, Suit::Spades, NO_TRUMPS_ADDITIONAL_CARD_WEIGHT),

				WeightPerRanksFromEachSuit::create({}, { Rank::Jack }, NO_TRUMPS_J_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Nine }, NO_TRUMPS_9_WEIGHT),
			},
			{}/*multiplicative*/
		};

		contractWeightManagers[Contract::Type::AllTrumps] = {
			{/*additive*/
				WeightPerRanksFromEachSuit::create({}, { Rank::Jack }, ALL_TRUMPS_J_WEIGHT),
				WeightPerRanksFromEachSuit::create({}, { Rank::Jack, Rank::Nine}, ALL_TRUMPS_J_9_WEIGHT),

				// 9 and 2 other cards
				Constant::create({ HaveAtLeastNCardsFromSuit::create(Suit::Clubs, 3), HaveCards::create({"9 Clubs"_c}) }, ALL_TRUMPS_9_AND_2_OTHER_CARDS_WEIGHT),
				Constant::create({ HaveAtLeastNCardsFromSuit::create(Suit::Diamonds, 3), HaveCards::create({"9 Diamonds"_c}) }, ALL_TRUMPS_9_AND_2_OTHER_CARDS_WEIGHT),
				Constant::create({ HaveAtLeastNCardsFromSuit::create(Suit::Hearts, 3), HaveCards::create({"9 Hearts"_c}) }, ALL_TRUMPS_9_AND_2_OTHER_CARDS_WEIGHT),
				Constant::create({ HaveAtLeastNCardsFromSuit::create(Suit::Spades, 3), HaveCards::create({"9 Spades"_c}) }, ALL_TRUMPS_9_AND_2_OTHER_CARDS_WEIGHT),

				// Additional cards from the suit
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Clubs"_c, "9 Clubs"_c}) }, Suit::Clubs, ALL_TRUMPS_ADDITIONAL_CARD_WEIGHT),
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Diamonds"_c, "9 Diamonds"_c}) }, Suit::Diamonds, ALL_TRUMPS_ADDITIONAL_CARD_WEIGHT),
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Hearts"_c, "9 Hearts"_c}) }, Suit::Hearts, ALL_TRUMPS_ADDITIONAL_CARD_WEIGHT),
				WeightPerEachCardFromSuit::create({ HaveCards::create({"J Spades"_c, "9 Spades"_c}) }, Suit::Spades, ALL_TRUMPS_ADDITIONAL_CARD_WEIGHT),
			},
			{}/*multiplicative*/
		};
	
		return contractWeightManagers;
	}
}


