#include "Trick.h"
#include "Contract.h"
#include "Round.h"
#include "Belote.h"
#include "Application.h"

bool Trick::canPlayCard(const Card& card) const
{
	if (m_turns.empty())
	{
		return true;
	}

	const Contract& contract = m_round->getBiddingManager().getContract();

	const Player& player = m_round->getBelote().getActivePlayer();
	const Card* firstCard = m_turns.front().m_card;
	const bool isFirstCardTrump = contract.isTrumpCard(*firstCard);

	const bool hasPlayerSameSuit = player.hasSuit(firstCard->getSuit());
	if (hasPlayerSameSuit)
	{
		if (card.getSuit() != firstCard->getSuit())
		{
			return false;
		}

		// Do we need to play stronger card
		if (isFirstCardTrump)
		{
			const Card* winnigCard = getWinningCard();

			const bool hasHigherRank = std::any_of(
				player.getCards().begin(),
				player.getCards().end(),
				[winnigCard](const Card* card)
				{
					return card->getSuit() == winnigCard->getSuit() && Contract::cardRankCompare(*card, *winnigCard, true) == 1;
				}
			);

			return Contract::cardRankCompare(card, *winnigCard, true) == 1 || !hasHigherRank;
		}

		return true;
	}

	// Player doesn't have the same suit
	if (isFirstCardTrump)
	{
		return true; // doesn't matter what will play
	}

	const bool isAllyWinning = getWinningCardTurn()->m_player->getTeamIndex() == player.getTeamIndex();
	if (isAllyWinning)
	{
		return true;
	}

	const Card* winningCard = getWinningCard();
	if (!contract.isTrumpCard(*winningCard)) // no trump played
	{
		const bool hasAnyTrumps = std::any_of(player.getCards().begin(), player.getCards().end(), [this, &contract](const Card* card)
		{
			return contract.isTrumpCard(*card);
		});

		return contract.isTrumpCard(card) || !hasAnyTrumps;
	}

	const bool isCardHigherTrump = card.getSuit() == winningCard->getSuit() && Contract::cardRankCompare(card, *winningCard, true) == 1;
	const bool hasPlayerHigherTrump = std::any_of(player.getCards().begin(), player.getCards().end(), [winningCard](const Card* card)
	{
		return card->getSuit() == winningCard->getSuit() && Contract::cardRankCompare(*card, *winningCard, true) == 1;
	});

	return isCardHigherTrump || !hasPlayerHigherTrump;
}

void Trick::playCard(const Card& card)
{
	if (!canPlayCard(card))
	{
		Utils::crashGame();
	}

	static_cast<Subject<NotifyCardAboutToBePlayed>&>(*Application::getInstance()).notifyObservers(NotifyCardAboutToBePlayed(m_round->getBelote().getActivePlayer(), card));
	
	m_turns.emplace_back(&card, &m_round->getBelote().getActivePlayer());

	if (isOver())
	{
		m_round->collectTrickCards();
	}
}

const TrickTurn* Trick::getWinningCardTurn() const
{
	if (m_turns.empty())
	{
		return nullptr;
	}

	const Contract& contract = m_round->getBiddingManager().getContract();

	const Card* winningCard = m_turns[0].m_card;
	size_t turnIndex = 0;
	for (size_t i = 1; i < m_turns.size(); ++i)
	{
		const Card* card = m_turns[i].m_card;
		if (contract.isTrumpCard(*winningCard))
		{
			if (card->getSuit() == winningCard->getSuit() && Contract::cardRankCompare(*card, *winningCard, true) == 1)
			{
				winningCard = card;
				turnIndex = i;
			}
		}
		else
		{
			if (card->getSuit() == winningCard->getSuit() && Contract::cardRankCompare(*card, *winningCard, false) == 1 ||
				contract.isTrumpCard(*card))
			{
				winningCard = card;
				turnIndex = i;
			}
		}
	}

	return &m_turns[turnIndex];
}

const Card* Trick::getWinningCard() const
{
	if (m_turns.empty())
	{
		return nullptr;
	}
	
	return getWinningCardTurn()->m_card;
}

std::vector<const Card*> Trick::getCards() const
{
	std::vector<const Card*> cards;
	for (const TrickTurn& turn : m_turns)
	{
		cards.push_back(turn.m_card);
	}
	return cards;
}
