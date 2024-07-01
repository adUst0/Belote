#pragma once
#include <memory>
#include <vector>

namespace MCTS
{
	class MCTSGameStateBase;

	class MCTSNode final
	{
	public:
		MCTSNode(std::unique_ptr<MCTSGameStateBase> state, MCTSNode* parent);

		bool									isFullyExpanded() const;

		bool									iseLeaf() const { return m_children.empty(); }

		void									expand();

		// Select the best child node based on the UCT formula (upper confidence bound applied to trees).
		MCTSNode*								bestChild(double explorationParameter);

		const MCTSGameStateBase&				getState() const { return *m_state; }
		const std::vector<MCTSNode>&			getChildren() const { return m_children; }
		std::vector<MCTSNode>&					getChildren() { return m_children; }
		MCTSNode*								getParent() { return m_parent; }

		size_t									getVisits() const { return m_visits; }
		void									incrementVisits() { ++m_visits; }

		double									getWins() const { return m_wins; }
		void									addWins(double value) { m_wins += value; }

	private:

		std::unique_ptr<MCTSGameStateBase>		m_state;

		size_t									m_visits = 0;
		double									m_wins = 0;

		std::vector<MCTSNode>					m_children;
		MCTSNode*								m_parent = nullptr;
	};
}