#include "MCTSTree.h"
#include <queue>
#include <set>
#include "MCTSNode.h"
#include "MCTSGameStateBase.h"
#include "MCTSMoveBase.h"

namespace MCTS
{
	MCTSTree::MCTSTree(std::unique_ptr<MCTSGameStateBase> initialState, size_t maxIterations /*= 1000*/)
		: m_root(std::move(initialState), nullptr)
		, m_maxIterations(maxIterations)
		, m_random_engine(std::random_device{}())
	{

	}

	MCTSNode* MCTSTree::runMCTS()
	{
		for (size_t i = 0; i < m_maxIterations; ++i)
		{
			MCTSNode* node = select(&m_root);
			node->expand();

			MCTSNode* leaf = node->iseLeaf() ? node : node->bestChild(0);
			const double result = simulate(leaf->getState());
			backpropagate(leaf, result);
		}

		return m_root.bestChild(0);
	}

	size_t MCTSTree::getTreeSize(bool expandEveryNode /*= false*/)
	{
		std::queue<MCTSNode*> q;
		std::set<MCTSNode*> visited;
		q.push(&m_root);
		size_t count = 1;
		while (!q.empty())
		{
			MCTSNode* node = q.front();
			visited.emplace(node);
			q.pop();

			if (expandEveryNode)
			{
				node->expand();
			}

			for (auto& child : node->getChildren())
			{
				if (visited.count(&child) == 0)
				{
					q.push(&child);
					++count;
				}

			}
		}

		return count;
	}

	MCTSNode* MCTSTree::select(MCTSNode* node)
	{
		while (!node->iseLeaf())
		{
			node = node->bestChild(m_explorationParameter);
		}

		return node;
	}

	double MCTSTree::simulate(const MCTSGameStateBase& state)
	{
		std::unique_ptr<MCTSGameStateBase> tempState = state.clone();

		while (!tempState->isTerminal())
		{
			MCTSMoveVector possibleMoves = tempState->generateAllPossibleMoves();
			const MCTSMoveBase& move = *possibleMoves[getRandomIndex(0, possibleMoves.size() - 1)];
			tempState->applyMove(move);
		}

		return tempState->getResult();
	}

	void MCTSTree::backpropagate(MCTSNode* node, double result)
	{
		while (node)
		{
			node->incrementVisits();
			node->addWins(result);
			node = node->getParent();
		}
	}

	size_t MCTSTree::getRandomIndex(size_t min, size_t max)
	{
		std::uniform_int_distribution<size_t> dis(min, max);
		return dis(m_random_engine);
	}

}
