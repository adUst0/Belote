#pragma once

class Player;
class Card;
enum class Contract : int8_t;

struct NotifyCardDealing
{
	const Player& m_player;
	const Card& m_card;
};

struct NotifyContractVote
{
	const Player& m_player;
	const Contract& m_contract;
};

struct NotifyCardAboutToBePlayed
{
	const Player& m_player;
	const Card& m_card;
};


struct NotifyEndOfTrick
{

};

struct NotifyEndOfRound
{

};