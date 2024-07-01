#pragma once
#include <vector>
#include <memory>

namespace MCTS
{
	struct MCTSMoveBase
	{
		MCTSMoveBase() = default;
		MCTSMoveBase(const MCTSMoveBase& other) = default;
		MCTSMoveBase& operator=(const MCTSMoveBase& other) = default;
		MCTSMoveBase(MCTSMoveBase&& other) = default;
		MCTSMoveBase& operator=(MCTSMoveBase&& other) = default;
		virtual ~MCTSMoveBase() = default;
	};

	using MCTSMoveVector = std::vector<std::unique_ptr<MCTSMoveBase>>;
}