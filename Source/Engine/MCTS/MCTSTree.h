#pragma once
#include <memory>
#include <vector>
#include "MCTSNode.h"
#include <random>

namespace MCTS
{
	class MCTSGameStateBase;

	class MCTSTree final
	{
	public:
		MCTSTree(std::unique_ptr<MCTSGameStateBase> initialState, size_t maxIterations = 1000);

		MCTSNode*					runMCTS();
		
		// Debug
		size_t						getTreeSize(bool expandEveryNode = false);

	private:

		// Select a node for expansion.
		MCTSNode*					select(MCTSNode* node);

		// Simulate a random playout from the given game state.
		double						simulate(const MCTSGameStateBase& state);

		// Backpropagate the result of a simulation up the tree
		void						backpropagate(MCTSNode* node, double result);

		size_t						getRandomIndex(size_t min, size_t max);

		double						m_explorationParameter = 1.41; //sqrt(2)
		size_t						m_maxIterations = 1000;

		MCTSNode					m_root;

		std::mt19937				m_random_engine;
	};
}