#pragma once
#include "Card.h"
#include <memory>
#include <vector>
#include "StateMachine.h"
#include <array>

// TODO: Move to separate files 

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

class Belote;

class Player
{
public:
	Player(int teamIndex, int playerIndex, Belote& belote);

	void										addCard(const Card& card) { m_cards.push_back(&card); }

	const std::vector<const Card*>&				getCards() const { return m_cards; }
	void										returnCards();

	int											getTeamIndex() const { return m_teamIndex; }
	int											getPlayerIndex() const { return m_playerIndex; }

	bool										isContractVoteRequired() const { return m_contractVoteRequired; }
	void										setContractVoteRequired();

	bool										isHuman() const { return m_isHuman; }

	const Belote*								getBelote() const { return m_belote; }

private:
	Belote*										m_belote = nullptr;

	bool										m_isHuman = false;

	int											m_teamIndex = 0;
	int											m_playerIndex = 0;

	bool										m_contractVoteRequired = false;

	std::vector<const Card*>					m_cards;
};




class Belote
{
public:
	Belote();

	enum class BeloteState : int8_t
	{
		StartNewGame = 0,
		DealCardsToActivePlayer,
		ChooseContract,
		PlayTrick, // One trick consists of all players playing a card
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
	void										returnCard(const Card& card) { m_deck.push_back(&card); }

	size_t										getActivePlayerIndex() const { return m_activePlayerIndex; }
	const Player&								getActivePlayer() const { return *m_players[m_activePlayerIndex]; }
	Player&										getActivePlayer() { return *m_players[m_activePlayerIndex]; }

	size_t										getNextPlayerIndex(size_t current = -1/*default is active player*/) const;  // counter-clockwise
	size_t										getPreviousPlayerIndex(size_t current = -1/*default is active player*/) const; // clockwise
	
	const Player&								getNextPlayer() const { return *m_players[getNextPlayerIndex()]; }
	Player&										getNextPlayer() { return *m_players[getNextPlayerIndex()]; }


	const std::vector<Contract>&				getContractVotes() const { return m_contractVotes; }
	void										voteForContract(Contract contract);
	bool										isValidContractVote(Contract vote) const;

	void										enterState(BeloteState state);
	void										update();

private:

	void										cutDeck();
	void										dealCardsToPlayer(Player& player, int numCards);

	ContractVoteData							getLastNonPassContractVote(bool ignoreDouble = true) const;
	Contract									decideContractFromVotes() const;

	// Belote States. TODO: This can be implemented using the StateMachine but I believe it would be overkill for now
	void										enterStartNewGameState();
	void										updateStartNewGameState();

	void										enterDealCardsToActivePlayerState();
	void										updateDealCardsToActivePlayerState();

	void										enterChooseContractState();
	void										updateChooseContractState();

	void										enterPlayTrickPhase();
	void										updatePlayTrickPhase();

	BeloteState									m_state = BeloteState::GameOver;


	std::vector<const Card*>					m_deck;
	std::vector<std::unique_ptr<Player>>		m_players;

	size_t										m_firstPlayerIndex = 0;
	size_t										m_activePlayerIndex = 0;


	std::vector<Contract>						m_contractVotes;
	Contract									m_contract = Contract::Num;

	static std::vector<std::unique_ptr<Card>>	s_cards;
};