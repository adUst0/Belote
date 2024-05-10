#include "Belote.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include <cassert>
#include "Player.h"
#include "Utils.h"
#include "Application.h"
#include <cmath>

namespace
{
	bool isRankAllowedInBelote(Rank rank)
	{
		switch (rank)
		{
		case Rank::Ace:
		case Rank::Seven:
		case Rank::Eight:
		case Rank::Nine:
		case Rank::Ten:
		case Rank::Jack:
		case Rank::Queen:
		case Rank::King:
			return true;
		default:
			return false;
		}
	}
}

std::vector<std::unique_ptr<Card>> Belote::s_cards;

Belote::Belote()
{
	// Initial creation of all Card objects
	if (s_cards.empty())
	{
		for (int i = 0; i < (int8_t)Suit::Num; ++i)
		{
			for (int j = 0; j < (int8_t)Rank::Num; ++j)
			{
				if (isRankAllowedInBelote(Rank(j)))
				{
					s_cards.emplace_back(std::make_unique<Card>(Suit(i), Rank(j)));
				}
			}
		}
	}

	for (int playerIndex = 0; playerIndex < 4; playerIndex++)
	{
		const int teamIndex = playerIndex % 2 == 0 ? 0 : 1;
		m_players.emplace_back(std::make_unique<Player>(teamIndex, playerIndex, *this));
	}

	for (auto& card_ptr : s_cards)
	{
		m_deck.emplace_back(card_ptr.get());
	}
	// Shuffle cards only for the first game
	std::shuffle(m_deck.begin(), m_deck.end(), std::default_random_engine((unsigned int)std::chrono::system_clock::now().time_since_epoch().count()));

	m_teamTotalScore[0] = m_teamTotalScore[1] = 0;

	enterState(BeloteState::StartNewGame);
}

size_t Belote::getNextPlayerIndex(size_t current /*= -1/*default is active player*/) const
{
	if (current == -1)
	{
		current = m_activePlayerIndex;
	}

	return current == m_players.size() - 1 ? 0 : current + 1;
}

size_t Belote::getPreviousPlayerIndex(size_t current /*= -1/*default is active player*/) const
{
	if (current == -1)
	{
		current = m_activePlayerIndex;
	}

	return current == 0 ? m_players.size() - 1 : current - 1;
}

void Belote::enterState(BeloteState state)
{
	m_state = state;
	Utils::log("ENTER {}. ActivePlayer is {} ({})\n", getStateString(), m_activePlayerIndex, getActivePlayer().isHuman() ? "human" : "AI");

	switch (state)
	{
	case Belote::BeloteState::StartNewGame:
		enterStartNewGameState();
		break;
	case Belote::BeloteState::DealCardsToActivePlayer:
		enterDealCardsToActivePlayerState();
		break;
	case Belote::BeloteState::ChooseContract:
		enterChooseContractState();
		break;
	case Belote::BeloteState::PlayCard:
		enterPlayCardPhase();
		break;
	case Belote::BeloteState::CollectTrickCardsAndUpdate:
		enterCollectTrickCardsAndUpdate();
		break;
	case Belote::BeloteState::CalculateEndOfRoundScore:
		enterCalculateEndOfRoundScore();
		break;
	case Belote::BeloteState::GameOver:
		enterGameOver();
		break;
	case Belote::BeloteState::Num:
		break;
	default:
		break;
	}
}

void Belote::updateState()
{
	Utils::log("UPDATE {}. ActivePlayer is {} ({})\n", getStateString(), m_activePlayerIndex, getActivePlayer().isHuman() ? "human" : "AI");

	switch (m_state)
	{
	case Belote::BeloteState::StartNewGame:
		updateStartNewGameState();
		break;
	case Belote::BeloteState::DealCardsToActivePlayer:
		updateDealCardsToActivePlayerState();
		break;
	case Belote::BeloteState::ChooseContract:
		updateChooseContractState();
		break;
	case Belote::BeloteState::PlayCard:
		updatePlayCardPhase();
		break;
	case Belote::BeloteState::CollectTrickCardsAndUpdate:
		updateCollectTrickCardsAndUpdate();
		break;
	case Belote::BeloteState::CalculateEndOfRoundScore:
		updateCalculateEndOfRoundScore();
		break;
	case Belote::BeloteState::GameOver:
		updateGameOver();
		break;
	case Belote::BeloteState::Num:
		break;
	default:
		break;
	}
}

std::string Belote::getStateString() const
{
	switch (m_state)
	{
	case Belote::BeloteState::StartNewGame:
		return "StartNewGame";
	case Belote::BeloteState::DealCardsToActivePlayer:
		return "DealCardsToActivePlayer";
	case Belote::BeloteState::ChooseContract:
		return "ChooseContract";
	case Belote::BeloteState::PlayCard:
		return "PlayTrick";
	case Belote::BeloteState::CollectTrickCardsAndUpdate:
		return "CollectTrickCardsAndUpdate";
	case Belote::BeloteState::CalculateEndOfRoundScore:
		return "CalculateEndOfRoundScore";
	case Belote::BeloteState::GameOver:
		return "GameOver";
	case Belote::BeloteState::Num:
		return "Num - INVALID";
	default:
		return "Missing string for state in operator<<";
	}
}

void Belote::cutDeck()
{
	// TODO
}

void Belote::dealCardsToPlayer(Player& player, int numCards)
{
	for (int i = 0; i < numCards; ++i)
	{
		const Card* card = m_deck.back();
		player.addCard(*card);
		m_deck.pop_back();

		Utils::log("Dealing card: {}\n", card->toString());
		static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).notifyObservers(NotifyCardDealing(player, *card));
	}
}

bool Belote::isGameOver() const
{
	return m_teamTotalScore[0] >= 151 || m_teamTotalScore[1] >= 151;
}

void Belote::createNewRound()
{
	m_rounds.emplace_back(std::make_unique<Round>(this));
	static_cast<Subject<NotifyNewRound>&>(*Application::getInstance()).notifyObservers(NotifyNewRound(getCurrentRound()));
}

void Belote::enterStartNewGameState()
{
	if (m_deck.size() != 32)
	{
		Utils::crashGame();
	}

	cutDeck();
}

void Belote::updateStartNewGameState()
{
	m_dealingPlayerIndex = getPreviousPlayerIndex(m_activePlayerIndex);
	createNewRound();

	enterState(BeloteState::DealCardsToActivePlayer);
}

void Belote::enterDealCardsToActivePlayerState()
{
	const int numCardsToDeal = getActivePlayer().getCards().size() == 3 ? 2 : 3;
	dealCardsToPlayer(getActivePlayer(), numCardsToDeal);
}

void Belote::updateDealCardsToActivePlayerState()
{
	const bool allPlayersHaveTheSameNumberOfCards = getNextPlayer().getCards().size() == getActivePlayer().getCards().size();

	m_activePlayerIndex = getNextPlayerIndex();

	if (allPlayersHaveTheSameNumberOfCards)
	{
		if (getActivePlayer().getCards().size() == 3)
		{
			enterState(BeloteState::DealCardsToActivePlayer); // deal the rest 2 cards of the initial hand
		}
		else if (getActivePlayer().getCards().size() == 5)
		{
			enterState(BeloteState::ChooseContract);
		}
		else
		{
			m_activePlayerIndex = getNextPlayerIndex(m_dealingPlayerIndex);
			enterState(BeloteState::PlayCard);
		}
	}
	else
	{
		enterState(BeloteState::DealCardsToActivePlayer);
	}
}

void Belote::enterChooseContractState()
{
	getActivePlayer().setContractVoteRequired();
	static_cast<Subject<NotifyContractVoteRequired>&>(*Application::getInstance()).notifyObservers(NotifyContractVoteRequired(getActivePlayer()));
}

void Belote::updateChooseContractState()
{
	const bool isActivePlayerReady = !getActivePlayer().isContractVoteRequired();
	if (!isActivePlayerReady)
	{
		// Still waiting
		return;
	}

	if (getCurrentRound().getBiddingManager().isBiddingOver())
	{
		// Go back to the original first player
		m_activePlayerIndex = getNextPlayerIndex();

		if (getCurrentRound().getBiddingManager().getContract().getType() == Contract::Type::Pass)
		{
			// TODO: do we want some specific order of returning cards?
			for (auto& player_ptr : m_players)
			{
				player_ptr->returnCards();
			}
		}

		enterState(BeloteState::DealCardsToActivePlayer);
	}
	else
	{
		m_activePlayerIndex = getNextPlayerIndex();
		enterState(BeloteState::ChooseContract);
	}
}

void Belote::enterPlayCardPhase()
{
	if (!getCurrentRound().anyTrickPlayed() || getCurrentRound().getCurrentTrick().isOver())
	{
		getCurrentRound().createNewTrick();
	}

	getActivePlayer().setPlayCardRequired();
}

void Belote::updatePlayCardPhase()
{
	const bool isActivePlayerReady = !getActivePlayer().isPlayCardRequired();
	if (!isActivePlayerReady)
	{
		// Still waiting
		return;
	}

	if (getCurrentRound().getCurrentTrick().isOver())
	{
		// Go back to the original first player
		m_activePlayerIndex = getNextPlayerIndex();

		enterState(BeloteState::CollectTrickCardsAndUpdate);
	}
	else
	{
		m_activePlayerIndex = getNextPlayerIndex();
		enterState(BeloteState::PlayCard);
	}
}


void Belote::enterCollectTrickCardsAndUpdate()
{
	static_cast<Subject<NotifyEndOfTrick>&>(*Application::getInstance()).notifyObservers(NotifyEndOfTrick(m_rounds.back()->getCurrentTrick()));

	m_activePlayerIndex = getCurrentRound().getCurrentTrick().getWinningCardTurn()->m_player->getPlayerIndex();

	Utils::log("enterCollectTrickCardsAndUpdate: winning player = {}\n", m_activePlayerIndex);
}

void Belote::updateCollectTrickCardsAndUpdate()
{
	if (getActivePlayer().getCards().empty())
	{
		enterState(BeloteState::CalculateEndOfRoundScore);
	}
	else
	{
		enterState(BeloteState::PlayCard);
	}
}

void Belote::enterCalculateEndOfRoundScore()
{
	m_teamTotalScore[0] = getCurrentRound().calculateTeamScore(0);
	m_teamTotalScore[1] = getCurrentRound().calculateTeamScore(1);
}

void Belote::updateCalculateEndOfRoundScore()
{
	auto&& playedCards = getCurrentRound().getPlayedCards();
	m_deck.insert(m_deck.end(), playedCards.begin(), playedCards.end());

	if (!isGameOver())
	{
		m_dealingPlayerIndex = getNextPlayerIndex(m_dealingPlayerIndex);
		m_activePlayerIndex = getNextPlayerIndex(m_dealingPlayerIndex);

		static_cast<Subject<NotifyEndOfRound>&>(*Application::getInstance()).notifyObservers(NotifyEndOfRound(*m_rounds.back()));
		createNewRound();

		enterState(BeloteState::DealCardsToActivePlayer);		
	}
	else
	{
		enterState(BeloteState::GameOver);
	}
}

void Belote::enterGameOver()
{
	Utils::log("GAME OVER: winning team is {}", m_teamTotalScore[0] > m_teamTotalScore[1] ? 0 : 1);
}

void Belote::updateGameOver()
{
	// TODO: start new game
}