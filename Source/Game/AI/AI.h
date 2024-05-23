#pragma once
#include "Belote.h"

class DummyAI
{
public:
	static Contract chooseContractVote(const Player& player);
	static const Card* chooseCardToPlay(const Player& player);
};

class AI
{
public:
	static Contract chooseContractVote(const Player& player);
	static const Card* chooseCardToPlay(const Player& player);

	static const Belote& getBelote() { return *s_belote; }
	static void setBelote(const Belote& belote) { s_belote = &belote; }

private:
	static inline const Belote* s_belote = nullptr;
};