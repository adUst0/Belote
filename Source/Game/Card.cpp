#include "Card.h"

#include <array>
#include <format>
#include <string>
#include <cassert>
#include <sstream>

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

std::array<std::string, (int8_t)Rank::Num> rankStrings2
{
	"a",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"10",
	"j",
	"q",
	"k"
};

const std::string& stringFromSuit(Suit suit)
{
	return (int8_t)suit >= (int8_t)Suit::Num ? empty_string : suitStrings[(int8_t)suit];
}

const std::string& stringFromRank(Rank rank)
{
	return (int8_t)rank >= (int8_t)Rank::Num ? empty_string : rankStrings[(int8_t)rank];
}

Suit suitFromString(const std::string& str)
{
	for (int8_t i = 0; i < (int8_t)Suit::Num; ++i)
	{
		if (str == suitStrings[i])
		{
			return Suit(i);
		}
	}

	return Suit::Num;
}

Rank rankFromString(const std::string& str)
{
	for (int8_t i = 0; i < (int8_t)Rank::Num; ++i)
	{
		if (str == rankStrings[i] || str == rankStrings2[i])
		{
			return Rank(i);
		}
	}

	return Rank::Num;
}

Card operator"" _c(const char* c, std::size_t)
{
	std::string cardStr(c);
	std::transform(cardStr.begin(), cardStr.end(), cardStr.begin(),
		[](unsigned char c) { return std::tolower(c); });

	std::stringstream ss(cardStr);

	std::string rankStr;
	std::getline(ss, rankStr, ' ');
	std::string suitStr;
	std::getline(ss, suitStr, ' ');

	Rank rank = rankFromString(rankStr);
	Suit suit = suitFromString(suitStr);

	if (rank == Rank::Num || suit == Suit::Num)
	{
		__debugbreak();
	}

	return { suit, rank };
}

Card::Card(Suit suit, Rank rank)
	: m_suit(suit)
	, m_rank(rank)
	, m_toString(std::format("Card {} {}", stringFromSuit(suit), stringFromRank(rank)))
	, m_texturePath(std::format("assets/{}_of_{}.jpg", stringFromRank(rank), stringFromSuit(suit)))
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

bool Card::operator==(const Card& rhs) const
{
	return m_suit == rhs.m_suit && m_rank == rhs.m_rank;
}
