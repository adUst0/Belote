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
};