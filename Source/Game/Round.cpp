#include "Round.h"
#include "Card.h"
#include "Player.h"

namespace
{
	const size_t NUM_TRICKS_PER_ROUND = 8;
}

Round::Round(const Belote* belote)
	: m_belote(belote)
{
	m_tricks.reserve(NUM_TRICKS_PER_ROUND);
}

void Round::createNewTrick()
{
	m_tricks.emplace_back(std::make_unique<Trick>(this));
}

size_t Round::getLastTrickWinnerTeam() const
{
	if (m_tricks.empty())
	{
		return std::numeric_limits<size_t>::max();
	}

	return m_tricks.back()->getWinningCardTurn()->m_player->getTeamIndex();
}

size_t Round::calculateTeamScore(size_t teamIndex) const
{
	// TODO: vytre
	// TODO: Declarations not implemented yet

	const Contract& contract = getBiddingManager().getContract();

	const size_t lastTrickWinnerTeam = getLastTrickWinnerTeam();
	int score = 0;
	for (const Card* card : m_teamCards[teamIndex])
	{
		score += card->getScore(contract.isTrumpCard(*card));
	}

	if (lastTrickWinnerTeam == teamIndex)
	{
		score += 10;
	}

	if (contract.getType() == Contract::Type::NoTrumps)
	{
		score *= 2;
	}

	const int ceilingPoint = contract.getType() == Contract::Type::AllTrumps ? 4 : (contract.getType() == Contract::Type::NoTrumps ? 5 : 6);
	score = (score % 10 >= ceilingPoint) ? std::ceil(score / 10.f) : std::floor(score / 10.f);

	score *= getBiddingManager().getContract().getScoreMultiplier();

	return score;
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
