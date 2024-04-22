#include "Belote.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include <cassert>
#include "Player.h"
#include "Utils.h"
#include "Application.h"
#include "States.h"

/* TODO Notes:
* 20 Apr 24, 17.25: done with Belote::isValidContractVote() but not tested
* 20 Apr 24, 23:14: done with contract voting. Todo: play tricks
*/

namespace
{
	std::string contractToString(Contract contract)
	{
		switch (contract)
		{
		case Contract::Pass:
			return "Pass";
			break;
		case Contract::Clubs:
			return "Clubs";
			break;
		case Contract::Diamonds:
			return "Diamonds";
			break;
		case Contract::Hearts:
			return "Hearts";
			break;
		case Contract::Spades:
			return "Spades";
			break;
		case Contract::NoTrumps:
			return "NoTrumps";
			break;
		case Contract::AllTrumps:
			return "AllTrumps";
			break;
		case Contract::Double:
			return "Double";
			break;
		case Contract::Redouble:
			return "Redouble";
			break;
		case Contract::Num:
			return "Num - Invalid";
			break;
		default:
			return "Missing string for Contract";
			break;
		}
	}

	std::string beloteStateToString(Belote::BeloteState state)
	{
		switch (state)
		{
		case Belote::BeloteState::StartNewGame:
			return "StartNewGame";
		case Belote::BeloteState::DealCardsToActivePlayer:
			return "DealCardsToActivePlayer";
		case Belote::BeloteState::ChooseContract:
			return "ChooseContract";
		case Belote::BeloteState::PlayTrick:
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

	std::ostream& operator<<(std::ostream& out, Belote::BeloteState state)
	{
		switch (state)
		{
		case Belote::BeloteState::StartNewGame:
			out << "StartNewGame";
			break;
		case Belote::BeloteState::DealCardsToActivePlayer:
			out << "DealCardsToActivePlayer";
			break;
		case Belote::BeloteState::ChooseContract:
			out << "ChooseContract";
			break;
		case Belote::BeloteState::PlayTrick:
			out << "PlayTrick";
			break;
		case Belote::BeloteState::CollectTrickCardsAndUpdate:
			out << "CollectTrickCardsAndUpdate";
			break;
		case Belote::BeloteState::CalculateEndOfRoundScore:
			out << "CalculateEndOfRoundScore";
			break;
		case Belote::BeloteState::GameOver:
			out << "GameOver";
			break;
		case Belote::BeloteState::Num:
			out << "Num - INVALID";
			break;
		default:
			out << "Missing string for state in operator<<()";
			break;
		}
		return out;
	}

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
		const int teamIndex = playerIndex % 2 != 0 ? 0 : 1;
		m_players.emplace_back(std::make_unique<Player>(teamIndex, playerIndex, *this));
	}

	for (auto& card_ptr : s_cards)
	{
		m_deck.emplace_back(card_ptr.get());
	}
	// Shuffle cards only for the first game
	std::shuffle(m_deck.begin(), m_deck.end(), std::default_random_engine((unsigned int)std::chrono::system_clock::now().time_since_epoch().count()));

	enterState(BeloteState::StartNewGame);
}

size_t Belote::getNextPlayerIndex(size_t current /*= -1/*default is active player*/) const
{
	if (current == -1)
	{
		current = m_activePlayerIndex;
	}

	return current == 0 ? m_players.size() - 1 : current - 1; // counter-clockwise
}

size_t Belote::getPreviousPlayerIndex(size_t current /*= -1/*default is active player*/) const
{
	if (current == -1)
	{
		current = m_activePlayerIndex;
	}

	return current == m_players.size() - 1 ? 0 : current + 1;
}

void Belote::voteForContract(Contract contract)
{
	assert(contract != Contract::Num);
	m_contractVotes.push_back(contract);
	Utils::log("Voting for contract {}\n", contractToString(contract));
}

bool Belote::isValidContractVote(Contract vote) const
{
	if (vote == Contract::Num)
	{
		return false;
	}

	if (vote == Contract::Pass)
	{
		return true;
	}

	const ContractVoteData current = getLastNonPassContractVote(true);
	const ContractVoteData currentWithDoubles = getLastNonPassContractVote(false);

	if (current.m_vote == Contract::Num)
	{
		return vote != Contract::Double && vote != Contract::Redouble;
	}

	if (currentWithDoubles.m_vote == Contract::Double)
	{
		const bool redouble = vote == Contract::Redouble && getActivePlayer().getTeamIndex() != currentWithDoubles.m_player->getTeamIndex();
		const bool escaping = vote != Contract::Double && (int8_t)vote > (int8_t)current.m_vote;
		return redouble || escaping;
	}
	else if (currentWithDoubles.m_vote == Contract::Redouble)
	{
		return false; // Only Pass available
	}

	if (vote == Contract::Redouble)
	{
		return false;
	}

	return (int8_t)vote > (int8_t)current.m_vote;
}

void Belote::enterState(BeloteState state)
{
	m_state = state;
	Utils::log("ENTER {}. ActivePlayer is {} ({})\n", beloteStateToString(m_state), m_activePlayerIndex, getActivePlayer().isHuman() ? "human" : "AI");

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
	case Belote::BeloteState::PlayTrick:
		enterPlayTrickPhase();
		break;
	case Belote::BeloteState::CollectTrickCardsAndUpdate:
		break;
	case Belote::BeloteState::CalculateEndOfRoundScore:
		break;
	case Belote::BeloteState::GameOver:
		break;
	case Belote::BeloteState::Num:
		break;
	default:
		break;
	}
}

void Belote::updateState()
{
	if (m_isStateMachinePaused)
	{
		return;
	}

	Utils::log("UPDATE {}. ActivePlayer is {} ({})\n", beloteStateToString(m_state), m_activePlayerIndex, getActivePlayer().isHuman() ? "human" : "AI");

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
	case Belote::BeloteState::PlayTrick:
		updatePlayTrickPhase();
		break;
	case Belote::BeloteState::CollectTrickCardsAndUpdate:
		break;
	case Belote::BeloteState::CalculateEndOfRoundScore:
		break;
	case Belote::BeloteState::GameOver:
		break;
	case Belote::BeloteState::Num:
		break;
	default:
		break;
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

		Utils::log("Dealing card: {} {}\n", stringFromRank(card->getRank()), stringFromSuit(card->getSuit()));
		//static_cast<GameState*>(Application::getInstance()->getStateMachine().getActiveState())->notifyCardDealing(player, *card);
		static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).notifyObservers(NotifyCardDealing(player, *card));
	}
}

Belote::ContractVoteData Belote::getLastNonPassContractVote(bool ignoreDoubles /*= true*/) const
{
	if (m_contractVotes.empty())
	{
		return { Contract::Num, nullptr };
	}

	size_t voteOwnerIndex = getPreviousPlayerIndex(m_activePlayerIndex);
	for (auto iter = m_contractVotes.rbegin(); iter != m_contractVotes.rend(); ++iter)
	{
		if (*iter != Contract::Pass && (*iter != Contract::Double && *iter != Contract::Redouble || !ignoreDoubles))
		{
			return { *iter, m_players[voteOwnerIndex].get() };
		}

		voteOwnerIndex = getPreviousPlayerIndex(voteOwnerIndex);
	}

	return { Contract::Num, nullptr };
}

Contract Belote::decideContractFromVotes() const
{
	if (m_contractVotes.size() < 4)
	{
		return Contract::Num;
	}

	// We need 3 passes to decide the contract
	for (int i = (int)m_contractVotes.size() - 1; i >= m_contractVotes.size() - 3; --i)
	{
		if (m_contractVotes[i] != Contract::Pass)
		{
			return Contract::Num;
		}
	}

	return m_contractVotes[m_contractVotes.size() - 4];
}

void Belote::enterStartNewGameState()
{
	if (m_deck.size() != 32)
	{
		__debugbreak();
		*(unsigned int*)0 = 0xDEADBEAF;
	}

	cutDeck();
}

void Belote::updateStartNewGameState()
{
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
			m_contract = Contract::Num;
			m_contractVotes.clear();

			enterState(BeloteState::ChooseContract);
		}
		else
		{
			enterState(BeloteState::PlayTrick);
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
}

void Belote::updateChooseContractState()
{
	const bool isActivePlayerReady = !getActivePlayer().isContractVoteRequired();
	if (!isActivePlayerReady)
	{
		// Still waiting
		return;
	}

	if (decideContractFromVotes() != Contract::Num)
	{
		m_contract = decideContractFromVotes();

		// Go back to the original first player
		m_activePlayerIndex = getNextPlayerIndex();

		if (m_contract == Contract::Pass)
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

void Belote::enterPlayTrickPhase()
{

}

void Belote::updatePlayTrickPhase()
{

}



/*
* 1. DEAL state:
*		Deal initial cards
* 2. BIDDING state:
*		On Enter state: set for all Players:
*			m_startedBidding = false;
*			m_endedBidding = false;
*
*		for each Player starting from StartPlayer
*			if (!Player.startedBidding())
*				Player.startBidding(); // Player.m_startedBidding = true; if AI then we make bidding and set m_endedBidding = true;
*
*			if (!Belote.didPlayerBid(Player)) or if (!Player.endedBidding())
*				break; // still waiting for that player.
*
*		biddingEnded = has 3 passes?
*		if (biddingEnded)
*			if hasContract
*				goto PLAY state
*			else
*				collect all cards
*				update first Player
*				go to DEAL state
*
* **********************
* Player:
*	-> lastBiddingContract = enum
*/