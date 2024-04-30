#pragma once
#include "Card.h"
#include <memory>
#include <vector>
#include "StateMachine.h"
#include <array>
#include "Player.h"
#include "BiddingManager.h"
#include "Round.h"

/* TODOs
* Implement card ownership with unique_ptr to enforce that the card really transfers ownership
* Round score bug: ceiling is incorrect. Got 19-8 score for all trumps
*/

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

	const std::vector<std::unique_ptr<Player>>& getPlayers() const { return m_players; }

	size_t										getDealingPlayerIndex() const { return m_dealingPlayerIndex; }

	size_t										getActivePlayerIndex() const { return m_activePlayerIndex; }
	const Player&								getActivePlayer() const { return *m_players[m_activePlayerIndex]; }
	Player&										getActivePlayer() { return *m_players[m_activePlayerIndex]; }

	size_t										getNextPlayerIndex(size_t current = -1/*default is active player*/) const;
	const Player&								getNextPlayer() const { return *m_players[getNextPlayerIndex()]; }
	Player&										getNextPlayer() { return *m_players[getNextPlayerIndex()]; }

	size_t										getPreviousPlayerIndex(size_t current = -1/*default is active player*/) const;

	const std::vector<const Card*>&				getDeck() const { return m_deck;}
	void										returnCardToDeck(const Card& card) { m_deck.push_back(&card); }
	
	Round&										getCurrentRound() { return *m_rounds.back(); }
	const Round&								getCurrentRound() const { return *m_rounds.back(); }

	int											getTeamScore(int teamIndex) const { return m_teamTotalScore[teamIndex]; }

	void										enterState(BeloteState state);
	void										updateState();

private:

	void										cutDeck();
	void										dealCardsToPlayer(Player& player, int numCards);
	bool										isGameOver() const;
	void										createNewRound();

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

	size_t										m_dealingPlayerIndex = 3;
	size_t										m_activePlayerIndex = 0;

	std::vector<std::unique_ptr<Round>>			m_rounds;

	int											m_teamTotalScore[2];

	static std::vector<std::unique_ptr<Card>>	s_cards;
};