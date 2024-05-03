#include "TGUIGameState.h"

#include "TGUISplashState.h"

#include "Application.h"
#include "GameState.h"
#include "TGUI/Backend/Window/SFML/BackendGuiSFML.hpp"
#include "TGUI/Widgets/Picture.hpp"
#include "TGUI/Widgets/Button.hpp"
#include "TGUI/Widgets/Label.hpp"

#include "TGUI/Widgets/Group.hpp"
#include "TGUI/Widgets/HorizontalWrap.hpp"
#include "TGUI/Widgets/HorizontalLayout.hpp"

namespace
{
	const unsigned int					SCREEN_WIDTH = 1600;
	const unsigned int					SCREEN_HEIGHT = 900;
	const sf::Vector2f					SCREEN_CENTER = { SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f };
	const float							SCREEN_PADDING = 35.f;

	const sf::Vector2f					CARD_SCALE{ 0.4f, 0.4f };
	const float							CARD_WIDTH = 222.f * CARD_SCALE.x;
	const float							CARD_HEIGHT = 323.f * CARD_SCALE.y;
	const float							VERTICAL_CARDS_OVERLAP = 0.66f;
	const float							HORIZONTAL_CARDS_OVERLAP = 0.f;

	const float							DECK_PADDING_OFFSET_PERCENT = 0.01f;
	tgui::Layout2d						DECK_POSITION{ "1%", "1%" };
	const float							DECK_CARDS_OFFSET = 9;


}

TGUIGameState::TGUIGameState(StateMachine& stateMachine)
	: BaseState(stateMachine)
	, m_gui(Application::getInstance()->getWindow())
{
	m_gui.add(tgui::Picture::create("assets/background.jpg"), "background");

	//createDeck();

	//createPlayerNames();

	testWidgetPositions();

}

void TGUIGameState::createPlayerNames()
{
	for (const auto& player_ptr : m_belote.getPlayers())
	{
		auto label = tgui::Label::create(player_ptr->getNameForUI());
		label->setTextSize(28);
		label->getRenderer()->setBackgroundColor("#33b249");
		label->getRenderer()->setBorders(1);
		const float offsetX = player_ptr->getPlayerIndex() == 2 ? label->getSize().x : 0;
		const float offsetY = player_ptr->getPlayerIndex() == 1 ? label->getSize().y : 0;

		label->setPosition(getPlayerNamePosition(*player_ptr, -offsetX, -offsetY));
		m_gui.add(label);
	}
}

void TGUIGameState::createDeck()
{
	size_t cardIndex = 0;
	for (const Card* card : m_belote.getDeck())
	{
		auto cardWidget = tgui::Picture::create(tgui::Texture(card->getTexturePath()));
		cardWidget->setScale(CARD_SCALE);
		cardWidget->setPosition(getCardPositionInDeck(cardIndex));
		m_gui.add(cardWidget, card->toString());
		++cardIndex;
	}
}

tgui::Layout2d TGUIGameState::getCardPositionInDeck(size_t cardIndex)
{
	const size_t maxCardsPerRow = (SCREEN_WIDTH - 2 * SCREEN_WIDTH * DECK_PADDING_OFFSET_PERCENT) / (CARD_WIDTH + DECK_CARDS_OFFSET);
	const size_t rowIndex = cardIndex / maxCardsPerRow;
	const size_t colIndex = cardIndex % maxCardsPerRow;

	tgui::Layout x = DECK_POSITION.x + colIndex * (CARD_WIDTH + DECK_CARDS_OFFSET);
	tgui::Layout y = DECK_POSITION.y + rowIndex * (CARD_HEIGHT + DECK_CARDS_OFFSET);
	return { x, y };
}

tgui::Layout2d TGUIGameState::getCardPositionInPlayer(const Player& player, size_t cardIndex)
{
	switch (player.getPlayerIndex())
	{
	case 0: // Left
		{
			tgui::Layout2d pos = getCardPositionInDeck(16);
			pos.y = pos.y + CARD_HEIGHT + SCREEN_PADDING + cardIndex * (CARD_HEIGHT * (1 - VERTICAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	case 1: // Bottom
		{
			tgui::Layout2d pos = getCardPositionInDeck(20);
			pos.y = SCREEN_HEIGHT - CARD_HEIGHT - SCREEN_PADDING;
			pos.x = pos.x + cardIndex * (CARD_WIDTH * (1 - HORIZONTAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	case 2: // Right
		{
			tgui::Layout2d pos = getCardPositionInDeck(31);
			pos.y = pos.y + CARD_HEIGHT + SCREEN_PADDING + cardIndex * (CARD_HEIGHT * (1 - VERTICAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	case 3: // Up
		{
			tgui::Layout2d pos = getCardPositionInDeck(20);
			pos.y = pos.y + CARD_HEIGHT + SCREEN_PADDING;
			pos.x = pos.x + cardIndex * (CARD_WIDTH * (1 - HORIZONTAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	}

	return {};
}

tgui::Layout2d TGUIGameState::getCardPositionOnTable(const Player& player)
{
	const Player& player1 = *m_belote.getPlayers()[1];
	const Player& player3 = *m_belote.getPlayers()[3];

	switch (player.getPlayerIndex())
	{
	case 0: // Left
	{
		tgui::Layout2d pos1 = getCardPositionInPlayer(player1, 2);
		tgui::Layout2d pos3 = getCardPositionInPlayer(player3, 2);
		pos1.y = pos3.y + (pos1.y - pos3.y) / 2;
		return pos1;
	}
	break;
	case 1: // Bottom
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player1, 3);
		pos.y = pos.y - CARD_HEIGHT - DECK_CARDS_OFFSET;
		pos.x = pos.x + (CARD_WIDTH + DECK_CARDS_OFFSET) / 2;
		return pos;
	}
	break;
	case 2: // Right
	{
		tgui::Layout2d pos1 = getCardPositionInPlayer(player1, 5);
		tgui::Layout2d pos3 = getCardPositionInPlayer(player3, 5);
		pos1.y = pos3.y + (pos1.y - pos3.y) / 2;
		return pos1;
	}
	break;
	case 3: // Up
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player3, 3);
		pos.y = pos.y + CARD_HEIGHT + DECK_CARDS_OFFSET;
		pos.x = pos.x + (CARD_WIDTH + DECK_CARDS_OFFSET) / 2;
		return pos;
	}
	break;
	}

	return {};
}

tgui::Layout2d TGUIGameState::getPlayerNamePosition(const Player& player, float offsetX /*= 0*/, float offsetY /*= 0*/)
{
	switch (player.getPlayerIndex())
	{
	case 0: // Left
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player, 4);
		pos.x = pos.x + CARD_WIDTH + DECK_CARDS_OFFSET + offsetX;
		return pos;
	}
	break;
	case 1: // Bottom
	{
		//tgui::Layout2d pos = getCardPositionOnTable(player);
		//pos.y = SCREEN_HEIGHT - SCREEN_PADDING;
		//return pos;
		tgui::Layout2d pos = getCardPositionInPlayer(player, 0);
		pos.y = pos.y - DECK_CARDS_OFFSET + offsetY;
		return pos;
	}
	break;
	case 2: // Right
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player, 4);
		pos.x = pos.x - DECK_CARDS_OFFSET + offsetX;
		return pos;
	}
	break;
	case 3: // Up
	{
		//tgui::Layout2d pos = getCardPositionOnTable(player);
		//pos.y = pos.y - CARD_HEIGHT - DECK_CARDS_OFFSET - SCREEN_PADDING;
		//return pos;
		tgui::Layout2d pos = getCardPositionInPlayer(player, 0);
		pos.y = pos.y + DECK_CARDS_OFFSET + CARD_HEIGHT;
		return pos;
	}
	break;
	}

	return {};
}

void TGUIGameState::handleInput()
{
	sf::RenderWindow& window = (sf::RenderWindow&)*m_gui.getTarget();

	sf::Event event;

	while (window.pollEvent(event))
	{
		m_gui.handleEvent(event);

		if (sf::Event::Closed == event.type)
		{
			window.close();
		}
		else if (sf::Event::KeyReleased == event.type && event.key.code == sf::Keyboard::Key::F5)
		{
			m_gui.removeAllWidgets();
			testWidgetPositions();
		}
	}
}

void TGUIGameState::draw()
{
	sf::RenderWindow& window = (sf::RenderWindow&)*m_gui.getTarget();

	window.clear({ 240, 240, 240 });
	m_gui.draw();
	window.display();
}

void TGUIGameState::testWidgetPositions()
{
	m_gui.add(tgui::Picture::create("assets/background.jpg"), "background");

	createDeck();
	createPlayerNames();

	size_t cardIndex = 0;
	for (const Card* card : m_belote.getDeck())
	{
		auto cardWidget = tgui::Picture::create(tgui::Texture(card->getTexturePath()));
		cardWidget->setScale(CARD_SCALE);
		const size_t playerIndex = cardIndex / 8;
		cardWidget->setPosition(getCardPositionInPlayer(*m_belote.getPlayers()[playerIndex], cardIndex % 8));
		m_gui.add(cardWidget, card->toString());
		++cardIndex;
	}
	for (auto& player_ptr : m_belote.getPlayers())
	{
		const Card* card = m_belote.getDeck()[player_ptr->getPlayerIndex()];
		auto cardWidget = tgui::Picture::create(tgui::Texture(card->getTexturePath()));
		cardWidget->setScale(CARD_SCALE);
		cardWidget->setPosition(getCardPositionOnTable(*player_ptr));
		m_gui.add(cardWidget, card->toString());
	}
}
