#include "MCTSGameStateBase.h"

namespace MCTS
{
	bool MCTS::MCTSGameStateBase::isTerminal() const
	{
		return generateAllPossibleMoves().empty();
	}
}
