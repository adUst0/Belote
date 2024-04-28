#include "Card.h"

#include <array>
#include <format>
#include <string>
#include <cassert>

namespace
{
	std::string empty_string;
}

std::array<std::string, (int8_t)Suit::Num> suitStrings
{
	"clubs",
	"diamonds",
	"hearts",
	"spades"
};

std::array<std::string, (int8_t)Rank::Num> rankStrings
{
	"ace",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"10",
	"jack",
	"queen",
	"king"
};

const std::string& stringFromSuit(Suit suit)
{
	return (int8_t)suit >= (int8_t)Suit::Num ? empty_string : suitStrings[(int8_t)suit];
}

const std::string& stringFromRank(Rank rank)
{
	return (int8_t)rank >= (int8_t)Rank::Num ? empty_string : rankStrings[(int8_t)rank];
}

Card::Card(Suit suit, Rank rank)
	: m_suit(suit)
	, m_rank(rank)
	, m_toString(std::format("Card {} {}", stringFromSuit(suit), stringFromRank(rank)))
{
}

int Card::getScore(bool isTrump) const
{
	switch (m_rank)
	{
	case Rank::Ace:
		return 11;
	case Rank::Seven:
	case Rank::Eight:
		return 0;
	case Rank::Nine:
		return isTrump ? 14 : 0;
	case Rank::Ten:
		return 10;
	case Rank::Jack:
		return isTrump ? 20 : 2;
	case Rank::Queen:
		return 3;
	case Rank::King:
		return 4;
	default:
		assert(false);
		return -1;
		break;
	}
}
