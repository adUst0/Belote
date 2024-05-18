#pragma once
#include <vector>
#include "Trick.h"
#include "Contract.h"
#include <memory>
#include "BiddingManager.h"

class Belote;

class Round
{
public:
	Round(const Belote* belote);

	const BiddingManager&				getBiddingManager() const { return m_biddingManager; }
	BiddingManager&						getBiddingManager() { return m_biddingManager; }

	void								createNewTrick();

	bool								anyTrickPlayed() const { return !m_tricks.empty(); }
	Trick&								getCurrentTrick() { return *m_tricks.back(); };
	const Trick&						getCurrentTrick() const { return *m_tricks.back(); };

	const Belote&						getBelote() const {return *m_belote;}

	size_t								getLastTrickWinnerTeam() const;
	size_t								calculatePointsFromCards(const std::vector<const Card*>& cards) const;
	std::pair<size_t, size_t>			calculateTeamScore() const;

	void								collectTrickCards();
	std::vector<const Card*>			getPlayedCards() const;

private:
	const Belote*						m_belote = nullptr;

	BiddingManager						m_biddingManager;

	std::vector<std::unique_ptr<Trick>> m_tricks;

	std::vector<const Card*>			m_teamCards[2];
};

