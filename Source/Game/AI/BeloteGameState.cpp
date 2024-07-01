#include "BeloteGameState.h"
#include "Belote.h"

BeloteGameState::BeloteGameState(const Round& round, const PlayerHandsArray& playerHands, size_t activePlayerIndex)
	: m_round(round)
	, m_playerHands(playerHands)
	, m_activePlayerIndex(activePlayerIndex)
{
	m_round.setIsSimulation(true);
}

double BeloteGameState::getResult() const
{
	return static_cast<double>(m_scores[m_teamIndex]);
}

MCTS::MCTSMoveVector BeloteGameState::generateAllPossibleMoves() const
{
	MCTS::MCTSMoveVector moves;
	for (const Card* card : getCurrentPlayerHand())
	{
		if (m_round.getCurrentTrick().canPlayCard(*card, m_round.getBelote().getPlayers()[m_activePlayerIndex].get()))
		{
			moves.emplace_back(std::make_unique<BeloteMove>(card));
		}
	}
	return moves;
}

void BeloteGameState::applyMove(const MCTS::MCTSMoveBase& move)
{
	const BeloteMove& beloteMove = static_cast<const BeloteMove&>(move);

	m_moveToThisState = beloteMove;

	m_playerHands[m_activePlayerIndex].erase(
		std::find(
			m_playerHands[m_activePlayerIndex].begin(),
			m_playerHands[m_activePlayerIndex].end(),
			beloteMove.m_card));

	m_round.getCurrentTrick().playCard(*beloteMove.m_card, m_round.getBelote().getPlayers()[m_activePlayerIndex].get());

	m_activePlayerIndex = (m_activePlayerIndex + 1) % 4;

	m_scores[0] += m_round.calculatePointsFromCards(m_round.getTeamCards(0));
	m_scores[1] += m_round.calculatePointsFromCards(m_round.getTeamCards(1));

	if (m_round.getCurrentTrick().isOver())
	{
		m_round.createNewTrick();
	}
}

std::unique_ptr<MCTS::MCTSGameStateBase> BeloteGameState::clone() const
{
	return std::make_unique<BeloteGameState>(*this);
}