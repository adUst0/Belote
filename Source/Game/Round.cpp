#include "Round.h"
#include "Card.h"
#include "Player.h"

namespace
{
	const size_t NUM_TRICKS_PER_ROUND = 8;

	const size_t ALL_TRICKS_WINNER_BONUS_SCORE = 9; // valat
}

Round::Round(const Belote* belote)
	: m_belote(belote)
{
	m_tricks.reserve(NUM_TRICKS_PER_ROUND);
}

Round::Round(const Round& other)
{
	m_belote = other.m_belote;

	m_biddingManager = other.m_biddingManager;

	m_tricks = other.m_tricks;
	for (Trick& trick : m_tricks)
	{
		trick.setRound(*this);
	}

	m_teamCards = other.m_teamCards;

	m_isSimulation = other.m_isSimulation;
}

void Round::createNewTrick()
{
	m_tricks.emplace_back(this, m_isSimulation);
}

size_t Round::getLastTrickWinnerTeam() const
{
	if (m_tricks.empty())
	{
		return std::numeric_limits<size_t>::max();
	}

	return m_tricks.back().getWinningCardTurn()->m_player->getTeamIndex();
}

size_t Round::calculatePointsFromCards(const std::vector<const Card*>& cards) const
{
	const Contract& contract = getBiddingManager().getContract();

	size_t score = 0;
	for (const Card* card : cards)
	{
		score += card->getScore(contract.isTrumpCard(*card));
	}

	return score;
}

std::pair<size_t, size_t> Round::calculateTeamScore() const
{
	// TODO: Declarations not implemented yet

	const Contract& contract = getBiddingManager().getContract();

	size_t score[] = { calculatePointsFromCards(m_teamCards[0]), calculatePointsFromCards(m_teamCards[1]) };

	const size_t lastTrickWinnerTeam = getLastTrickWinnerTeam();
	score[lastTrickWinnerTeam] += 10;

	if (contract.getType() == Contract::Type::NoTrumps)
	{
		score[0] *= 2;
		score[1] *= 2;
	}

	// Vytre
	const size_t winningTeam = score[0] >= score[1] ? 0 : 1;
	if (contract.getPlayer()->getTeamIndex() != winningTeam)
	{
		score[winningTeam] += score[contract.getPlayer()->getTeamIndex()];
		score[contract.getPlayer()->getTeamIndex()] = 0;
	}

	const size_t totalPoints = std::round((score[0] + score[1]) / 10.f);

	const int ceilingPoint = contract.getType() == Contract::Type::AllTrumps ? 4 : (contract.getType() == Contract::Type::NoTrumps ? 5 : 6);
	score[0] = (score[0] % 10 >= ceilingPoint) ? std::ceil(score[0] / 10.f) : std::floor(score[0] / 10.f);
	score[1] = (score[1] % 10 >= ceilingPoint) ? std::ceil(score[1] / 10.f) : std::floor(score[1] / 10.f);

	// Fix rounding issue
	if (score[0] + score[1] > totalPoints)
	{
		size_t& smaller = score[0] < score[1] ? score[0] : score[1];
		--smaller;
	}

	for (size_t i = 0; i < 2; ++i)
	{
		if (m_teamCards[i].size() == 32)
		{
			score[i] += ALL_TRICKS_WINNER_BONUS_SCORE;
			break;
		}
	}

	score[0] *= getBiddingManager().getContract().getScoreMultiplier();
	score[1] *= getBiddingManager().getContract().getScoreMultiplier();

	return { score[0], score[1] };
}

void Round::collectTrickCards()
{
	if (!anyTrickPlayed())
	{
		return;
	}

	const size_t winningTeam = getLastTrickWinnerTeam();
	for (const Card* card : getCurrentTrick().getCards())
	{
		m_teamCards[winningTeam].push_back(card);
	}
}

std::vector<const Card*> Round::getPlayedCards() const
{
	auto result = m_teamCards[0];
	result.insert(result.end(), m_teamCards[1].begin(), m_teamCards[1].end());
	return result;
}

void Round::setIsSimulation(bool value)
{
	m_isSimulation = value;

	for (auto& trick : m_tricks)
	{
		trick.setIsSimulation(value);
	}
}
