#include "BiddingManager.h"
#include <cassert>
#include "Utils.h"
#include "Application.h"
#include "Player.h"

void BiddingManager::bid(Contract&& contract)
{
	assert(contract.getType() != Contract::Type::Invalid);
	m_biddings.emplace_back(std::move(contract));

	Utils::log("Voting for contract {}\n", m_biddings.back().toString());
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).notifyObservers(NotifyContractVote(*m_biddings.back().getPlayer(), m_biddings.back()));
}

void BiddingManager::bid(const Contract& contract)
{
	Contract temp = contract;
	bid(std::move(temp));
}

bool BiddingManager::canBid(const Contract& bid) const
{
	if (bid.getType() == Contract::Type::Invalid)
	{
		return false;
	}

	if (bid.getType() == Contract::Type::Pass)
	{
		return true;
	}

	const Contract& currentContract = getContract();

	if (currentContract.getType() == Contract::Type::Pass || currentContract.getType() == Contract::Type::Invalid)
	{
		return bid.getLevel() == Contract::Level::Normal;
	}

	if (bid.getLevel() != Contract::Level::Normal)
	{
		return bid.getType() != Contract::Type::Pass && 
			currentContract.getType() != Contract::Type::Pass && 
			(int8_t)bid.getLevel() == (int8_t)currentContract.getLevel() + 1 &&
			bid.getPlayer()->getTeamIndex() != currentContract.getPlayer()->getTeamIndex();
	}

	return (int8_t)bid.getType() > (int8_t)currentContract.getType();
}

bool BiddingManager::isBiddingOver() const
{
	if (m_biddings.size() < 4)
	{
		return false;
	}

	// We need 3 passes to decide the contract
	for (int i = m_biddings.size() - 1; i >= m_biddings.size() - 3; --i)
	{
		if (m_biddings[i].getType() != Contract::Type::Pass)
		{
			return false;
		}
	}

	return true;
}

const Contract& BiddingManager::getContract() const
{
	static const Contract invalidContract{ Contract::Type::Invalid, nullptr };

	if (m_biddings.empty())
	{
		return invalidContract;
	}

	// Search for the last non-pass vote
	for (auto it = m_biddings.rbegin(); it != m_biddings.rend(); ++it)
	{
		if (it->getType() != Contract::Type::Pass)
		{
			return *it;
		}
	}

	// Return the last pass vote
	return m_biddings.back();
}