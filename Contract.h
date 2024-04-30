#pragma once
#include <string>

class Player;
class Card;

class Contract
{
public:
	enum class Type { Pass = 0, Clubs, Diamonds, Hearts, Spades, NoTrumps, AllTrumps, Invalid };
	enum class Level { Normal = 0, Double, Redouble };

	Contract(Type type, const Player* player, Level level = Level::Normal)
		: m_type(type), m_player(player), m_level(level) {}

	Type							getType() const { return m_type; }
	Level							getLevel() const { return m_level; }
	const Player*					getPlayer() const { return m_player; }

	int								getScoreMultiplier() const;

	std::string						toString() const;

	bool							isTrumpCard(const Card& card) const;

	static int						cardRankCompare(const Card& lhs, const Card& rhs, bool trump);

private:
	Type							m_type;
	Level							m_level;
	const Player*					m_player;
};

