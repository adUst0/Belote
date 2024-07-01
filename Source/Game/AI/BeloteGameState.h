#pragma once
#include "MCTS/MCTSGameStateBase.h"
#include "Round.h"

class Card;

struct BeloteMove : public MCTS::MCTSMoveBase
{
	BeloteMove(const Card* card = nullptr) : m_card(card) { }
	const Card* m_card;
};

using PlayerHandsArray = std::array<std::vector<const Card*>, 4>;

class BeloteGameState : public MCTS::MCTSGameStateBase
{
public:
	BeloteGameState(const Round& round, const PlayerHandsArray& playerHands, size_t activePlayerIndex);

	double										getResult() const override;

	MCTS::MCTSMoveVector						generateAllPossibleMoves() const override;

	void										applyMove(const MCTS::MCTSMoveBase& move) override;

	std::unique_ptr<MCTS::MCTSGameStateBase>	clone() const override;

	const BeloteMove&							getMoveToThisState() const { return m_moveToThisState; }

private:

	const std::vector<const Card*>&			getCurrentPlayerHand() const { return m_playerHands[m_activePlayerIndex]; }

	Round									m_round;
	PlayerHandsArray						m_playerHands;

	std::array<size_t, 2>					m_scores = { 0, 0 };
	size_t									m_teamIndex = 0; // doesn't change. this is the team that we run the MCTS for. TODO: better name

	size_t									m_activePlayerIndex = 0;

	BeloteMove								m_moveToThisState;
};

