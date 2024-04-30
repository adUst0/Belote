#pragma once
#include "Contract.h"
#include <vector>

class Card;

class BiddingManager
{
public:
	void							bid(Contract&& contract);
	bool							canBid(const Contract& contract) const;

	bool							isBiddingOver() const;

	const Contract&					getContract() const;

private:
	std::vector<Contract>			m_biddings;
};

