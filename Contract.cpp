#include "Contract.h"
#include "Card.h"

int Contract::getScoreMultiplier() const
{
	switch (m_level)
	{
	case Contract::Level::Double:
		return 2;
	case Contract::Level::Redouble:
		return 4;
	default:
		return 1;
	}
}

std::string Contract::toString() const
{
	std::string str;

	switch (m_type)
	{
	case Type::Pass:
		str = "Pass";
		break;
	case Type::Clubs:
		str = "Clubs";
		break;
	case Type::Diamonds:
		str = "Diamonds";
		break;
	case Type::Hearts:
		str = "Hearts";
		break;
	case Type::Spades:
		str = "Spades";
		break;
	case Type::NoTrumps:
		str = "NoTrumps";
		break;
	case Type::AllTrumps:
		str = "AllTrumps";
		break;
	case Type::Invalid:
		str = "Invalid";
		break;
	default:
		str = "Missing string for Contract";
		break;
	}

	if (m_level == Level::Double)
	{
		str += " (Double)";
	}
	else if (m_level == Level::Redouble)
	{
		str += " (Redouble)";
	}

	return str;
}

bool Contract::isTrumpCard(const Card& card) const
{
	switch (m_type)
	{
	case Type::Clubs:
		return card.getSuit() == Suit::Clubs;
	case Type::Diamonds:
		return card.getSuit() == Suit::Diamonds;
	case Type::Hearts:
		return card.getSuit() == Suit::Hearts;
	case Type::Spades:
		return card.getSuit() == Suit::Spades;
	case Type::NoTrumps:
		return false;
	case Type::AllTrumps:
		return true;
	default:
		return false;
	}
}

int Contract::cardRankCompare(const Card& lhs, const Card& rhs, bool trump)
{
	static const std::vector<Rank> trumpOrder{ Rank::Seven, Rank::Eight, Rank::Queen, Rank::King, Rank::Ten, Rank::Ace, Rank::Nine, Rank::Jack };
	static const std::vector<Rank> noTrumpOrder{ Rank::Seven, Rank::Eight, Rank::Nine, Rank::Jack, Rank::Queen, Rank::King, Rank::Ten, Rank::Ace, };

	const std::vector<Rank>& order = trump ? trumpOrder : noTrumpOrder;

	if (lhs.getRank() == rhs.getRank())
	{
		return 0;
	}

	const int lhsIndex = std::distance(order.begin(), std::find(order.begin(), order.end(), lhs.getRank()));
	const int rhsIndex = std::distance(order.begin(), std::find(order.begin(), order.end(), rhs.getRank()));

	return lhsIndex < rhsIndex ? -1 : 1;
}
