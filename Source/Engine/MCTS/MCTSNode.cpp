#include "MCTSNode.h"
#include "MCTSTree.h"
#include "MCTSGameStateBase.h"
#include "MCTSMoveBase.h"

namespace MCTS
{
	MCTSNode::MCTSNode(std::unique_ptr<MCTSGameStateBase> state, MCTSNode* parent)
		: m_state(std::move(state))
		, m_parent(parent)
	{}

	bool MCTSNode::isFullyExpanded() const
	{
		MCTSMoveVector possibleMoves = m_state->generateAllPossibleMoves();
		return m_children.size() == possibleMoves.size();
	}

	void MCTSNode::expand()
	{
		if (isFullyExpanded())
		{
			return;
		}

		MCTSMoveVector possibleMoves = m_state->generateAllPossibleMoves();
		m_children.reserve(possibleMoves.size());

		for (const auto& move : possibleMoves)
		{
			std::unique_ptr<MCTSGameStateBase> newState = m_state->clone();
			newState->applyMove(*move);
			m_children.emplace_back(std::move(newState), this);
		}
	}

	MCTSNode* MCTSNode::bestChild(double explorationParameter)
	{
		MCTSNode* bestChild = nullptr;
		double bestValue = -std::numeric_limits<double>::infinity();

		for (MCTSNode& child : m_children)
		{
			const double ucbValue = (child.m_wins / (child.m_visits + 1)) + explorationParameter * sqrt(log((double)m_visits + 1) / (child.m_visits + 1));
			if (ucbValue > bestValue)
			{
				bestValue = ucbValue;
				bestChild = &child;
			}
		}

		return bestChild;
	}

}
