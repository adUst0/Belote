#pragma once
#include <memory>
#include "MCTSTree.h"
#include "MCTSMoveBase.h"

namespace MCTS
{
	struct MCTSMoveBase;

	class MCTSGameStateBase
	{
	public:
		MCTSGameStateBase() = default;
		MCTSGameStateBase(const MCTSGameStateBase& other) = default;
		MCTSGameStateBase& operator=(const MCTSGameStateBase& other) = default;
		MCTSGameStateBase(MCTSGameStateBase&& other) = default;
		MCTSGameStateBase& operator=(MCTSGameStateBase&& other) = default;
		virtual ~MCTSGameStateBase() = default;

		virtual double							getResult() const = 0;
		virtual MCTSMoveVector					generateAllPossibleMoves() const = 0;
		virtual bool							isTerminal() const;

		virtual void							applyMove(const MCTSMoveBase& move) = 0;

		virtual std::unique_ptr<MCTSGameStateBase>	clone() const = 0;
	};
}

