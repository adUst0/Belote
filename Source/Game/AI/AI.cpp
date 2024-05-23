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

using CardInfo = std::pair<Rank, Suit>;

namespace BiddingConditions
{
	class BiddingConditionBase
	{
	public:
		BiddingConditionBase(bool negate = false) : m_negate(negate) {}
		virtual ~BiddingConditionBase() = default;

		bool operator()() const {
			return m_negate ? !checkCondition() : checkCondition();
		}

	private:
		virtual bool checkCondition() const = 0;

		bool m_negate = false;
	};

	class HaveCards : public BiddingConditionBase
	{
	public:
		HaveCards(const std::vector<CardInfo>& cards, bool negate = false) : BiddingConditionBase(negate), m_cards(cards) {}

		static std::shared_ptr<BiddingConditionBase> create(const std::vector<CardInfo>& cards, bool negate = false)
		{
			std::shared_ptr<BiddingConditionBase> ptr = std::make_shared<HaveCards>(cards, negate);
			return ptr;
		}

	private:
		bool checkCondition() const override
		{
			const auto& activePlayerCards = AI::getBelote().getActivePlayer().getCards();
			return std::all_of(m_cards.begin(), m_cards.end(), [&activePlayerCards](const CardInfo& cardInfo)
			{
				return std::find_if(activePlayerCards.begin(), activePlayerCards.end(), [&cardInfo](const Card* card)
				{
					return card->getRank() == cardInfo.first &&
						card->getSuit() == cardInfo.second;
				}) != activePlayerCards.end();
			});
		}

		std::vector<CardInfo> m_cards;
	};

	class HaveAtLeastNCardsFromSuit : public BiddingConditionBase
	{
	public:
		HaveAtLeastNCardsFromSuit(Suit suit, size_t numCards, bool negate = false)
			: BiddingConditionBase(negate)
			, m_suit(suit)
			, m_numCards(numCards)
		{}

		template <typename...Params>
		static std::shared_ptr<BiddingConditionBase> create(Params&&... params)
		{
			std::shared_ptr<BiddingConditionBase> ptr = std::make_shared<HaveAtLeastNCardsFromSuit>(std::forward<Params>(params)...);
			return ptr;
		}
	private:
		bool checkCondition() const override
		{
			const auto& activePlayerCards = AI::getBelote().getActivePlayer().getCards();
			return std::count_if(activePlayerCards.begin(), activePlayerCards.end(), [suit = m_suit](const Card* card)
			{
				return card->getSuit() == suit;
			}) >= (int)m_numCards;
		}

		Suit m_suit;
		size_t m_numCards;
	};
}

namespace BiddingWeightGenerator
{
	using PreconditionsVec = std::vector<std::shared_ptr<BiddingConditions::BiddingConditionBase>>;

	class BiddingWeightGeneratorBase
	{
	public:
		BiddingWeightGeneratorBase(const PreconditionsVec& preconditions)
			: m_preconditions(preconditions)
		{}

		virtual ~BiddingWeightGeneratorBase() = default;

		float generateWeight() const
		{
			for (auto& condition : m_preconditions)
			{
				if (!(*condition)())
				{
					return 0.f;
				}
			}

			return generateWeightImpl();
		}
	
	protected:
		virtual float generateWeightImpl() const = 0;

		PreconditionsVec m_preconditions;
	};

	class Constant : public BiddingWeightGeneratorBase
	{
	public:
		Constant(const PreconditionsVec& preconditions, float value)
			: BiddingWeightGeneratorBase(preconditions)
			, m_value(value)
		{}

		template <typename...Params>
		static std::shared_ptr<BiddingWeightGeneratorBase> create(Params&&... params)
		{
			std::shared_ptr<BiddingWeightGeneratorBase> ptr = std::make_shared<Constant>(std::forward<Params>(params)...);
			return ptr;
		}

	private:
		float generateWeightImpl() const { return m_value; }

		float m_value;
	};
}

using GeneratorVec = std::vector<std::shared_ptr<BiddingWeightGenerator::BiddingWeightGeneratorBase>>;

class ContractWeight
{
public:
	ContractWeight() = default;
	ContractWeight(const GeneratorVec& additiveGenerators, const GeneratorVec& multiplicativeGenerators)
		: m_additiveGenerators(additiveGenerators)
		, m_multiplicativeGenerators(multiplicativeGenerators)
	{

	}

	float calculateWeight() const
	{
		float weight = 0;

		for (auto& generator : m_additiveGenerators)
		{
			weight += generator->generateWeight();
		}

		float multiplier = 1.f;
		for (auto& generator : m_multiplicativeGenerators)
		{
			multiplier += generator->generateWeight();
		}

		weight *= multiplier;

		return weight;
	}

private:
	GeneratorVec m_additiveGenerators;
	GeneratorVec m_multiplicativeGenerators;

};

CardInfo operator"" _c(const char* c, std::size_t)
{
	char buff[101];
	strcpy_s(buff, 100, c);

	Rank rank(Rank::Seven);
	Suit suit(Suit::Clubs);

	char* nextToken = nullptr;
	char* nextToken2 = nullptr;
	const char* token = strtok_s(buff, " ", &nextToken);
	if (token)
	{
		if (strcmp(token, "7") == 0)
		{
			rank = Rank::Seven;
		}
		else if (strcmp(token, "8") == 0)
		{
			rank = Rank::Eight;
		}
		else if (strcmp(token, "9") == 0)
		{
			rank = Rank::Nine;
		}
		else if (strcmp(token, "10") == 0)
		{
			rank = Rank::Ten;
		}
		else if (strcmp(token, "J") == 0)
		{
			rank = Rank::Jack;
		}
		else if (strcmp(token, "Q") == 0)
		{
			rank = Rank::Queen;
		}
		else if (strcmp(token, "K") == 0)
		{
			rank = Rank::King;
		}
		else if (strcmp(token, "A") == 0)
		{
			rank = Rank::Ace;
		}
		else
		{
			__debugbreak();
		}
	}
	else __debugbreak();

	token = strtok_s(nextToken, " ", &nextToken2);
	if (token)
	{
		if (strcmp(token, "Clubs") == 0)
		{
			suit = Suit::Clubs;
		}
		else if (strcmp(token, "Diamonds") == 0)
		{
			suit = Suit::Diamonds;
		}
		else if (strcmp(token, "Hearts") == 0)
		{
			suit = Suit::Hearts;
		}
		else if (strcmp(token, "Spades") == 0)
		{
			suit = Suit::Spades;
		}
		else
		{
			__debugbreak();
		}
	}
	else __debugbreak();

	return { rank, suit };
}

std::map<Contract::Type, ContractWeight> createContractWeights()
{
	std::map<Contract::Type, ContractWeight> contractWeights;

	using namespace BiddingConditions;
	using namespace BiddingWeightGenerator;

	contractWeights[Contract::Type::Pass] = {
		{/*additive*/
			Constant::create(PreconditionsVec{}, 50.f)
		},
		{}/*multiplicative*/
	};

	contractWeights[Contract::Type::Clubs] = ContractWeight{
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

	contractWeights[Contract::Type::Diamonds] = ContractWeight{
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

	contractWeights[Contract::Type::Hearts] = ContractWeight{
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

	contractWeights[Contract::Type::Spades] = ContractWeight{
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

	return contractWeights;
}


Contract AI::chooseContractVote(const Player& player)
{
	static std::map<Contract::Type, ContractWeight> contractWeights = createContractWeights();

	std::vector<std::pair<Contract, float>> contracts;

	for (auto& [contractType, contractWeight] : contractWeights)
	{
		Contract contract(contractType, &player);
		if (AI::getBelote().getCurrentRound().getBiddingManager().canBid(contract))
		{
			contracts.emplace_back(std::move(contract), contractWeight.calculateWeight());
		}
	}

	if (!contracts.empty())
	{
		std::sort(contracts.begin(), contracts.end(), [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
		return contracts.back().first;
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