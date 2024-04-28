#pragma once
#include "Card.h"
#include <memory>
#include <vector>
#include "StateMachine.h"
#include <array>
#include "Player.h"

// TODO: Implement card ownership with unique_ptr to enforce that the card really transfers ownership

// TODO: Contract can go into a class
enum class Contract : int8_t
{
	Pass = 0,
	Clubs,
	Diamonds,
	Hearts,
	Spades,
	NoTrumps,
	AllTrumps,
	Double,
	Redouble,
	Num,
};

bool isTrumpSuit(Suit suit, Contract currentContract);

std::string contractToString(Contract contract);

class Belote
{
public:
	Belote();

	enum class BeloteState : int8_t
	{
		StartNewGame = 0,
		ChooseContract,
		DealCardsToActivePlayer,
		PlayCard, // One trick consists of all players playing a card
		CollectTrickCardsAndUpdate,
		CalculateEndOfRoundScore, // One round consists of 8 tricks
		GameOver,
		Num
	};

	struct ContractVoteData
	{
		Contract m_vote = Contract::Num;
		Player* m_player = nullptr;
	};

	const std::vector<std::unique_ptr<Player>>& getPlayers() const { return m_players; }
	const std::vector<const Card*>&				getDeck() const { return m_deck;}
	void										returnCardToDeck(const Card& card) { m_deck.push_back(&card); }

	size_t										getActivePlayerIndex() const { return m_activePlayerIndex; }
	const Player&								getActivePlayer() const { return *m_players[m_activePlayerIndex]; }
	Player&										getActivePlayer() { return *m_players[m_activePlayerIndex]; }

	size_t										getNextPlayerIndex(size_t current = -1/*default is active player*/) const;
	size_t										getPreviousPlayerIndex(size_t current = -1/*default is active player*/) const;

	size_t										getDealingPlayerIndex() const { return m_dealingPlayerIndex; }
	
	const Player&								getNextPlayer() const { return *m_players[getNextPlayerIndex()]; }
	Player&										getNextPlayer() { return *m_players[getNextPlayerIndex()]; }

	const std::vector<Contract>&				getContractVotes() const { return m_contractVotes; }
	void										voteForContract(Contract contract);
	bool										isValidContractVote(Contract vote) const;

	bool										isValidCardToPlay(const Card& card) const;
	//std::vector<const Card*>					getValidCardsToPlay()
	void										playCard(const Card& card);

	// Trick functions
	const Player*								getCurrentPlayerWinningTrick() const;
	const Card*									getTrickHighestTrumpPlayed() const;

	const std::vector<const Card*>&				getCurrentTrickCards() const { return m_currentTrickCards; }

	int											getTeamScore(int teamIndex) const { return m_teamTotalScore[teamIndex]; }

	void										enterState(BeloteState state);
	void										updateState();

	// TODO: this needs to be reworked for double/redouble
	Contract									getContract() const { return m_contract; }
	const Player*								getContractPlayer() const { getLastNonPassContractVote(true).m_player; } // todo: very bad name


private:

	void										cutDeck();
	void										dealCardsToPlayer(Player& player, int numCards);

	ContractVoteData							getLastNonPassContractVote(bool ignoreDouble = true) const;
	Contract									decideContractFromVotes() const;

	int											calculateEndOfRoundScoreFromCards(size_t teamIndex) const;

	bool										isGameOver() const;

	// Belote States. This can be implemented using the StateMachine class but it would be overkill for now
	void										enterStartNewGameState();
	void										updateStartNewGameState();

	void										enterDealCardsToActivePlayerState();
	void										updateDealCardsToActivePlayerState();

	void										enterChooseContractState();
	void										updateChooseContractState();

	void										enterPlayCardPhase();
	void										updatePlayCardPhase();

	void										enterCollectTrickCardsAndUpdate();
	void										updateCollectTrickCardsAndUpdate();

	void										enterCalculateEndOfRoundScore();
	void										updateCalculateEndOfRoundScore();

	void										enterGameOver();
	void										updateGameOver();

	BeloteState									m_state = BeloteState::GameOver;


	std::vector<const Card*>					m_deck;
	std::vector<std::unique_ptr<Player>>		m_players;

	std::vector<const Card*>					m_currentTrickCards;

	size_t										m_dealingPlayerIndex = 3;
	size_t										m_activePlayerIndex = 0;


	std::vector<Contract>						m_contractVotes;
	Contract									m_contract = Contract::Num;

	struct RoundScore
	{
		std::vector<const Card*> m_teamCollectedCardsThisRound[2];
		size_t m_lastTrickWinningTeam = 0;
	};

	RoundScore									m_roundScore;

	int											m_teamTotalScore[2];

	static std::vector<std::unique_ptr<Card>>	s_cards;
};