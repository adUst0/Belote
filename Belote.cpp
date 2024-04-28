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

bool isTrumpSuit(Suit suit, Contract currentContract)
{
	switch (currentContract)
	{
	case Contract::Clubs:
		return suit == Suit::Clubs;
		break;
	case Contract::Diamonds:
		return suit == Suit::Diamonds;
		break;
	case Contract::Hearts:
		return suit == Suit::Hearts;
		break;
	case Contract::Spades:
		return suit == Suit::Spades;
		break;
	case Contract::NoTrumps:
		return false;
		break;
	case Contract::AllTrumps:
		return true;
		break;
	default:
		return false;
		break;
	}
}

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

namespace
{
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
		case Belote::BeloteState::PlayCard:
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

	int trumpRankCompare(Rank lhs, Rank rhs)
	{
		static const std::vector<Rank> order { Rank::Seven, Rank::Eight, Rank::Queen, Rank::King, Rank::Ten, Rank::Ace, Rank::Nine, Rank::Jack };

		if (lhs == rhs)
		{
			return 0;
		}

		const int lhsIndex = std::distance(order.begin(), std::find(order.begin(), order.end(), lhs));
		const int rhsIndex = std::distance(order.begin(), std::find(order.begin(), order.end(), rhs));

		return lhsIndex < rhsIndex ? -1 : 1;
	}

	int noTrumpRankCompare(Rank lhs, Rank rhs)
	{
		static const std::vector<Rank> order{ Rank::Seven, Rank::Eight, Rank::Nine, Rank::Jack, Rank::Queen, Rank::King, Rank::Ten, Rank::Ace, };

		if (lhs == rhs)
		{
			return 0;
		}

		const int lhsIndex = std::distance(order.begin(), std::find(order.begin(), order.end(), lhs));
		const int rhsIndex = std::distance(order.begin(), std::find(order.begin(), order.end(), rhs));

		return lhsIndex < rhsIndex ? -1 : 1;
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

void Belote::voteForContract(Contract contract)
{
	assert(contract != Contract::Num);
	m_contractVotes.push_back(contract);
	Utils::log("Voting for contract {}\n", contractToString(contract));
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).notifyObservers(NotifyContractVote(getActivePlayer(), contract));
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

bool Belote::isValidCardToPlay(const Card& card) const
{
	if (m_currentTrickCards.empty())
	{
		return true;
	}

	const Player& player = getActivePlayer();
	const Card* firstCard = m_currentTrickCards[0];
	const bool isFirstCardTrump = isTrumpSuit(firstCard->getSuit(), m_contract);

	const bool hasPlayerSameSuit = std::any_of(getActivePlayer().getCards().begin(), getActivePlayer().getCards().end(), [firstCard](const Card* card) { return card->getSuit() == firstCard->getSuit(); });
	if (hasPlayerSameSuit)
	{
		if (card.getSuit() != firstCard->getSuit())
		{
			return false;
		}

		// Do we need to play stronger card
		if (isFirstCardTrump)
		{
			const Card* winnigCard = getTrickHighestTrumpPlayed();

			const bool hasHigherRank = std::any_of(
				getActivePlayer().getCards().begin(),
				getActivePlayer().getCards().end(),
				[rank = winnigCard->getRank(), suit = winnigCard->getSuit()](const Card* card)
			{
				return card->getSuit() == suit && trumpRankCompare(card->getRank(), rank) == 1;
			}
			);

			return trumpRankCompare(card.getRank(), winnigCard->getRank()) == 1 || !hasHigherRank;
		}

		return true;
	}

	// Player doesn't have the same suit
	if (isFirstCardTrump)
	{
		return true; // doesn't matter what will play
	}

	const bool isAllyWinning = getCurrentPlayerWinningTrick()->getTeamIndex() == getActivePlayer().getTeamIndex();
	if (isAllyWinning)
	{
		return true;
	}

	const Card* highestTrumpPlayed = getTrickHighestTrumpPlayed();
	if (!highestTrumpPlayed) // no trump played
	{
		const bool hasAnyTrumps = std::any_of(getActivePlayer().getCards().begin(), getActivePlayer().getCards().end(), [this](const Card* card)
		{
			return isTrumpSuit(card->getSuit(), m_contract);
		});

		return isTrumpSuit(card.getSuit(), m_contract) || !hasAnyTrumps;
	}

	const bool isCardHigherTrump = card.getSuit() == highestTrumpPlayed->getSuit() && trumpRankCompare(card.getRank(), highestTrumpPlayed->getRank()) == 1;
	const bool hasPlayerHigherTrump = std::any_of(getActivePlayer().getCards().begin(), getActivePlayer().getCards().end(), [highestTrumpPlayed](const Card* card)
	{
		return card->getSuit() == highestTrumpPlayed->getSuit() && trumpRankCompare(card->getRank(), highestTrumpPlayed->getRank()) == 1;
	});
	
	return isCardHigherTrump || !hasPlayerHigherTrump;
}

void Belote::playCard(const Card& card)
{
	static_cast<Subject<NotifyCardAboutToBePlayed>&>(*Application::getInstance()).notifyObservers(NotifyCardAboutToBePlayed(getActivePlayer(), card));
	m_currentTrickCards.push_back(&card);
}

const Player* Belote::getCurrentPlayerWinningTrick() const
{
	if (m_currentTrickCards.empty())
	{
		return nullptr;
	}

	if (m_currentTrickCards.size() == 1)
	{
		m_players[getPreviousPlayerIndex()].get();
	}

	size_t playerIndex = getActivePlayerIndex();
	for (const Card* card : m_currentTrickCards)
	{
		playerIndex = getPreviousPlayerIndex(playerIndex);
	}

	const Card* winnigCard = m_currentTrickCards[0];
	size_t winningCardIndex = 0;
	for (size_t i = 1; i < m_currentTrickCards.size(); ++i)
	{
		const Card* card = m_currentTrickCards[i];
		if (isTrumpSuit(winnigCard->getSuit(), m_contract))
		{
			if (card->getSuit() == winnigCard->getSuit() && trumpRankCompare(card->getRank(), winnigCard->getRank()) == 1)
			{
				winnigCard = card;
				winningCardIndex = i;
			}
		}
		else
		{
			if (card->getSuit() == winnigCard->getSuit() && noTrumpRankCompare(card->getRank(), winnigCard->getRank()) == 1 ||
				isTrumpSuit(card->getSuit(), m_contract))
			{
				winnigCard = card;
				winningCardIndex = i;
			}
		}
	}

	while (winningCardIndex > 0)
	{
		--winningCardIndex;
		playerIndex = getNextPlayerIndex(playerIndex);
	}

	return m_players[playerIndex].get();
}

const Card* Belote::getTrickHighestTrumpPlayed() const
{
	if (m_currentTrickCards.empty())
	{
		return nullptr;
	}

	const Card* winnigCard = m_currentTrickCards[0];
	size_t winningCardIndex = 0;
	for (size_t i = 1; i < m_currentTrickCards.size(); ++i)
	{
		const Card* card = m_currentTrickCards[i];
		if (isTrumpSuit(winnigCard->getSuit(), m_contract))
		{
			if (card->getSuit() == winnigCard->getSuit() && trumpRankCompare(card->getRank(), winnigCard->getRank()) == 1)
			{
				winnigCard = card;
				winningCardIndex = i;
			}
		}
		else
		{
			if (card->getSuit() == winnigCard->getSuit() && noTrumpRankCompare(card->getRank(), winnigCard->getRank()) == 1 ||
				isTrumpSuit(card->getSuit(), m_contract))
			{
				winnigCard = card;
				winningCardIndex = i;
			}
		}
	}
	if (isTrumpSuit(winnigCard->getSuit(), m_contract))
	{
		return winnigCard;
	}
	else
	{
		return nullptr;
	}
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

int Belote::calculateEndOfRoundScoreFromCards(size_t teamIndex) const
{
	int score = 0;

	for (const Card* card : m_roundScore.m_teamCollectedCardsThisRound[teamIndex])
	{
		score += card->getScore(isTrumpSuit(card->getSuit(), m_contract));
	}

	if (m_roundScore.m_lastTrickWinningTeam == teamIndex)
	{
		score += 10;
	}

	// TODO: Declarations not implemented yet

	return m_contract == Contract::NoTrumps ? score * 2 : score;
}

bool Belote::isGameOver() const
{
	return m_teamTotalScore[0] >= 151 || m_teamTotalScore[1] >= 151;
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
	m_dealingPlayerIndex = getPreviousPlayerIndex(m_activePlayerIndex);
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
			m_activePlayerIndex = getNextPlayerIndex();
			enterState(BeloteState::DealCardsToActivePlayer); // deal the rest 2 cards of the initial hand
		}
		else if (getActivePlayer().getCards().size() == 5)
		{
			m_contract = Contract::Num;
			m_contractVotes.clear();

			m_activePlayerIndex = getNextPlayerIndex();
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

void Belote::enterPlayCardPhase()
{
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

	if (m_currentTrickCards.size() == 4)
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
	static_cast<Subject<NotifyEndOfTrick>&>(*Application::getInstance()).notifyObservers(NotifyEndOfTrick());

	const size_t winningTeamIndex = getCurrentPlayerWinningTrick()->getTeamIndex();
	m_roundScore.m_teamCollectedCardsThisRound[winningTeamIndex].insert(m_roundScore.m_teamCollectedCardsThisRound[winningTeamIndex].end(), m_currentTrickCards.begin(), m_currentTrickCards.end());
	m_roundScore.m_lastTrickWinningTeam = winningTeamIndex;

	m_activePlayerIndex = getCurrentPlayerWinningTrick()->getPlayerIndex();

	m_currentTrickCards.clear();

	Utils::log("enterCollectTrickCardsAndUpdate: winning team = {}\n", winningTeamIndex);
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
	for (size_t teamIndex = 0; teamIndex < 2; ++teamIndex)
	{
		int ceilingPoint = m_contract == Contract::AllTrumps ? 4 : (m_contract == Contract::NoTrumps ? 5 : 6);

		const auto scoreFromCards = calculateEndOfRoundScoreFromCards(teamIndex);
		const auto score = (scoreFromCards % 10 >= ceilingPoint) ? std::ceil(scoreFromCards / 10.f) : std::floor(scoreFromCards / 10.f);

		Utils::log("End of Round score for Team {} is: {}\n", teamIndex, score);

		m_teamTotalScore[teamIndex] += score;
	}

	// TODO: Vytre i kontra/rekontra
}

void Belote::updateCalculateEndOfRoundScore()
{
	if (!isGameOver())
	{
		m_dealingPlayerIndex = getNextPlayerIndex(m_dealingPlayerIndex);
		m_activePlayerIndex = getNextPlayerIndex(m_dealingPlayerIndex);

		m_deck.insert(m_deck.end(), m_roundScore.m_teamCollectedCardsThisRound[0].begin(), m_roundScore.m_teamCollectedCardsThisRound[0].end());
		m_deck.insert(m_deck.end(), m_roundScore.m_teamCollectedCardsThisRound[1].begin(), m_roundScore.m_teamCollectedCardsThisRound[1].end());
		m_roundScore.m_teamCollectedCardsThisRound[0].clear();
		m_roundScore.m_teamCollectedCardsThisRound[1].clear();

		enterState(BeloteState::DealCardsToActivePlayer);

		static_cast<Subject<NotifyEndOfRound>&>(*Application::getInstance()).notifyObservers(NotifyEndOfRound());
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