#pragma once
#include <map>
#include "Contract.h"
#include "AI/BiddingWeightGeneratorManager.h"

namespace AI::AIConfig
{
	std::map<Contract::Type, BiddingWeightGeneratorManager> createBiddingWeightGeneratorManagers();
}

