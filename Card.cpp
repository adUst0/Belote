#include "Card.h"

#include <array>
#include <format>
#include <string>

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
{
	
}

std::ostream& operator<<(std::ostream& out, const Card& card)
{
	out << stringFromSuit(card.getSuit()) << " " << stringFromRank(card.getRank());
	return out;
}