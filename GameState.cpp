#include "GameState.h"
#include "Application.h"
#include "UIComponent.h"

namespace
{
	struct PlayerPosition
	{
		sf::Vector2f m_position;
		bool m_isHorizontallyCentered = false;
		bool m_isReversedDirection = false;
	};

	const float							WAIT_TIME_AFTER_BIDDING = 0.5f;
	const float							WAIT_TIME_AFTER_PLAYING = 1.f;

	const unsigned int					SCREEN_WIDTH = 1600;
	const unsigned int					SCREEN_HEIGHT = 900;
	const sf::Vector2f					SCREEN_CENTER = { SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f };
	const float							SCREEN_PADDING = 20.f;

	const sf::Vector2f					CARD_SCALE{ 0.5f, 0.5f };
	const float							CARD_WIDTH = 222.f * CARD_SCALE.x;
	const float							CARD_HEIGHT = 323.f * CARD_SCALE.y;
	const float							CARD_OVERLAP_IN_HAND_PERCENT = 0.33f;

	const std::vector<PlayerPosition>	PLAYER_POSITIONS {
		{ {SCREEN_PADDING, SCREEN_CENTER.y}, false, false }, // left
		{ {SCREEN_CENTER.x, SCREEN_HEIGHT - SCREEN_PADDING - CARD_HEIGHT}, true, false }, // bottom
		{ {SCREEN_WIDTH - SCREEN_PADDING - CARD_WIDTH, SCREEN_CENTER.y}, false, true }, // right
		{ {SCREEN_CENTER.x, SCREEN_PADDING}, true, false } // up
	};

	static const sf::Vector2f			PLAYER_NAME_POSITIONS[4] {
		{SCREEN_PADDING, SCREEN_CENTER.y - 50.f}, // left
		{SCREEN_CENTER.x, SCREEN_HEIGHT - SCREEN_PADDING - CARD_HEIGHT - 50.f}, // bottom
		{SCREEN_WIDTH - SCREEN_PADDING - CARD_WIDTH * 8 * (1 - CARD_OVERLAP_IN_HAND_PERCENT), SCREEN_CENTER.y - 50.f}, // right
		{SCREEN_CENTER.x, SCREEN_PADDING + CARD_HEIGHT}, // up
	};

	const sf::Vector2f					DECK_POSITION = SCREEN_CENTER;

	static const sf::Vector2f			CARD_POSITIONS_ON_TABLE[4] { // Current trick cards
		{SCREEN_CENTER.x - CARD_WIDTH, SCREEN_CENTER.y}, // left
		{SCREEN_CENTER.x, SCREEN_CENTER.y + CARD_HEIGHT}, // top
		{SCREEN_CENTER.x + CARD_WIDTH, SCREEN_CENTER.y}, // right
		{SCREEN_CENTER.x, SCREEN_CENTER.y - CARD_HEIGHT}, // bottom
	};
}

GameState::GameState(StateMachine& stateMachine)
	: UIState(stateMachine)
{
	UIComponent* background = getOrCreateComponent("background");
	background->addSprite("assets/background.jpg");

	createCardSprites();
	createPlayerNames();

	UIComponent* pauseText = getOrCreateComponent("pause");
	pauseText->setText("GAME PAUSED!", sf::Color::Red, 72);
	pauseText->setOriginCenter(true);
	pauseText->setVisible(m_shouldPauseGame);
	pauseText->setPosition(DECK_POSITION);

	static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyCardAboutToBePlayed>&>(*Application::getInstance()).registerObserver(*this);
}

void GameState::createCardSprites()
{
	for (const Card* card : m_belote.getDeck())
	{
		UIComponent* component = getOrCreateComponent(card->toString());
		const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(card->getRank()), stringFromSuit(card->getSuit()));
		component->addSprite(path, CARD_SCALE);
		component->setBackground(sf::Color::White);
		component->setPosition(DECK_POSITION);
		component->setVisible(false);
	}

	UIComponent* cardBack = getOrCreateComponent("card_back");
	cardBack->addSprite("assets/card_back.png", CARD_SCALE);
	cardBack->setPosition(DECK_POSITION);
}

void GameState::createPlayerNames()
{
	for (const auto& player_ptr : m_belote.getPlayers())
	{
		UIComponent* component = getOrCreateComponent(std::format("player_{}", player_ptr->getPlayerIndex()));
		component->setText(getPlayerName(*player_ptr));
		component->setPosition(PLAYER_NAME_POSITIONS[player_ptr->getPlayerIndex()]);
		component->onMouseLeftClick([key = component->getKey()](){
			std::cout << "YOU CLICKED ME: " << key << std::endl;
		});
	}
}

sf::Vector2f GameState::calculateCardPosition(const Player& player, int cardOrder) const
{
	const float direction = PLAYER_POSITIONS[player.getPlayerIndex()].m_isReversedDirection ? -1 : 1;
	float positionX = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.x + (cardOrder * CARD_WIDTH * (1 - CARD_OVERLAP_IN_HAND_PERCENT)) * direction;
	if (PLAYER_POSITIONS[player.getPlayerIndex()].m_isHorizontallyCentered)
	{
		positionX -= (player.getCards().size() / 2.f) * CARD_WIDTH * (1 - CARD_OVERLAP_IN_HAND_PERCENT);
	}

	float positionY = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.y;
	return { positionX, positionY };
}

sf::Vector2f GameState::getCardPositionOnTable(const Player& player) const
{
	return CARD_POSITIONS_ON_TABLE[player.getPlayerIndex()];
}

std::string GameState::getPlayerName(const Player& player) const
{
	return std::format("Player {} ({})", player.getPlayerIndex(), player.isHuman() ? "human" : "AI");
}

void GameState::handleInput()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	sf::Event event;

	while (window.pollEvent(event))
	{
		if (sf::Event::Closed == event.type)
		{
			window.close();
		}
		else if (sf::Event::MouseButtonReleased == event.type && event.mouseButton.button == sf::Mouse::Button::Left)
		{
			// Start in reverse order because the last element is rendered on top
			for (auto rit = m_uiComponents.rbegin(); rit != m_uiComponents.rend(); ++rit)
			{
				if ((*rit)->handleLeftMouseClick({ (float)event.mouseButton.x, (float)event.mouseButton.y }))
				{
					break;
				}
			}
		}
		else if (sf::Event::KeyReleased == event.type && event.key.code == sf::Keyboard::Key::F5)
		{
			togglePause();
		}
	}
}

void GameState::update(float dtSeconds)
{
	UIState::update(dtSeconds);

	if (m_delayGameSeconds > 0.f)
	{
		m_delayGameSeconds -= dtSeconds;
	}

	const bool shouldPause = m_delayGameSeconds > 0.f || m_shouldPauseGame || std::any_of(m_uiComponents.begin(), m_uiComponents.end(), [](auto& item) { return item->isMoving(); });
	if (!shouldPause)
	{
		m_belote.updateState();
	}
}

void GameState::draw()
{
	UIState::draw();
}

void GameState::notify(const NotifyCardDealing& data)
{
	const auto targetPosition = calculateCardPosition(data.m_player, data.m_player.getCards().size() - 1);

	UIComponent* card = getComponent(data.m_card.toString());
	card->moveToPosition(targetPosition, 1.f);
	card->setVisible(true);

	if (m_belote.getDeck().empty())
	{
		getComponent("card_back")->setVisible(false);
	}
}

void GameState::notify(const NotifyContractVote& data)
{
	const std::string key = std::format("player {} vote:", data.m_player.getPlayerIndex());
	UIComponent* voteText = getComponent(key);

	if (!voteText)
	{
		voteText = getOrCreateComponent(key);
		voteText->setText(key + contractToString(data.m_contract));
		voteText->setPosition({ 0.f, 36.f * data.m_player.getPlayerIndex() });
	}
	else
	{
		voteText->setText(voteText->getText() + ", " + contractToString(data.m_contract));
	}

	delayGame(WAIT_TIME_AFTER_BIDDING);
}

void GameState::notify(const NotifyCardAboutToBePlayed& data)
{
	getComponent(data.m_card.toString())->moveToPosition(getCardPositionOnTable(data.m_player));

	// Update card in hands position
	size_t index = 0;
	for (const Card* card : data.m_player.getCards())
	{
		if (card == &data.m_card) continue;
		//getComponent(card->toString())->setPosition(calculateCardPosition(data.m_player, index++));
		getComponent(card->toString())->moveToPosition(calculateCardPosition(data.m_player, index++), 0.4f);
	}

	delayGame(WAIT_TIME_AFTER_PLAYING);
}

void GameState::delayGame(float seconds)
{
	m_delayGameSeconds = seconds;
}

void GameState::togglePause()
{
	m_shouldPauseGame = !m_shouldPauseGame;

	getComponent("pause")->setVisible(m_shouldPauseGame);
}