#include "GameState.h"
#include "Application.h"
#include "UIComponent.h"
#include "Contract.h"

namespace
{
	struct PlayerPosition
	{
		sf::Vector2f m_position;
		enum {Vertical, Horizontal} m_direction;
	};

	const float							WAIT_TIME_AFTER_BIDDING = 0.5f;
	const float							WAIT_TIME_AFTER_PLAYING = 1.f;
	const float							CARD_DEALING_TIME_SECONDS = 0.5;

	const unsigned int					SCREEN_WIDTH = 1600;
	const unsigned int					SCREEN_HEIGHT = 900;
	const sf::Vector2f					SCREEN_CENTER = { SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f };
	const float							SCREEN_PADDING = 20.f;

	const sf::Vector2f					CARD_SCALE{ 0.5f, 0.5f };
	const float							CARD_WIDTH = 222.f * CARD_SCALE.x;
	const float							CARD_HEIGHT = 323.f * CARD_SCALE.y;
	const float							CARD_OVERLAP_IN_HAND_PERCENT = 0.5f;
	const float							CARD_OVERLAP_ON_TABLE_PERCENT = 0.4f;

	const std::vector<PlayerPosition>	PLAYER_POSITIONS{
		{ {SCREEN_PADDING + CARD_WIDTH/2.f, SCREEN_CENTER.y}, PlayerPosition::Vertical}, // left
		{ {SCREEN_CENTER.x, SCREEN_HEIGHT - SCREEN_PADDING*2 - CARD_HEIGHT}, PlayerPosition::Horizontal }, // bottom
		{ {SCREEN_WIDTH - SCREEN_PADDING - CARD_WIDTH/2.f, SCREEN_CENTER.y}, PlayerPosition::Vertical }, // right
		{ {SCREEN_CENTER.x, SCREEN_PADDING*2}, PlayerPosition::Horizontal } // up
	};

	static const sf::Vector2f			PLAYER_NAME_POSITIONS[4] {
		{SCREEN_PADDING * 3 + CARD_WIDTH, SCREEN_CENTER.y}, // left
		{SCREEN_CENTER.x, SCREEN_HEIGHT - SCREEN_PADDING}, // bottom
		{SCREEN_WIDTH - SCREEN_PADDING * 3 - CARD_WIDTH, SCREEN_CENTER.y}, // right
		{SCREEN_CENTER.x, SCREEN_PADDING}, // up
	};

	static const sf::Vector2f			PLAYER_BIDDING_TEXT_POSITIONS[4]{
		{SCREEN_PADDING * 3 + CARD_WIDTH + 50.f, SCREEN_CENTER.y}, // left
		{SCREEN_CENTER.x, SCREEN_HEIGHT - SCREEN_PADDING - CARD_HEIGHT - 100.f}, // bottom
		{SCREEN_WIDTH - SCREEN_PADDING * 3 - CARD_WIDTH - 200.f, SCREEN_CENTER.y}, // right
		{SCREEN_CENTER.x, SCREEN_PADDING + CARD_HEIGHT + 50.f}, // up
	};

	const sf::Vector2f					DECK_POSITION = SCREEN_CENTER;

	static const sf::Vector2f			CARD_POSITIONS_ON_TABLE[4] { // Current trick cards
		{SCREEN_CENTER.x - CARD_WIDTH*(1- CARD_OVERLAP_ON_TABLE_PERCENT), SCREEN_CENTER.y - 80.f}, // left
		{SCREEN_CENTER.x, SCREEN_CENTER.y + CARD_HEIGHT*(1- CARD_OVERLAP_ON_TABLE_PERCENT) - 80.f}, // top
		{SCREEN_CENTER.x + CARD_WIDTH*(1- CARD_OVERLAP_ON_TABLE_PERCENT), SCREEN_CENTER.y - 80.f}, // right
		{SCREEN_CENTER.x, SCREEN_CENTER.y - CARD_HEIGHT*(1- CARD_OVERLAP_ON_TABLE_PERCENT) - 80.f}, // bottom
	};

	const sf::Vector2f					SCORE_POSITION;
	const sf::Vector2f					FIRST_PLAYER_TEXT_POSITION{0.f, 40.f};
}

GameState::GameState(StateMachine& stateMachine)
	: BaseState(stateMachine)
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

	UIComponent* scoreText = getOrCreateComponent("score");
	scoreText->setText(std::format("Score: {} - {}", m_belote.getTeamScore(0), m_belote.getTeamScore(1)));
	scoreText->setPosition(SCORE_POSITION);

	UIComponent* firstPlayerText = getOrCreateComponent("first_player");
	firstPlayerText->setText(std::format("First player this round: {}", getPlayerName(*m_belote.getPlayers()[m_belote.getNextPlayerIndex(m_belote.getDealingPlayerIndex())])));
	firstPlayerText->setPosition(FIRST_PLAYER_TEXT_POSITION);

	// TODO: double/redouble needs to be reworked
	//UIComponent* contractText = getOrCreateComponent("contract");
	//contractText->setText(std::format("Contract: {} ({})", contractToString(m_belote.getContract()), m_belote.getTeamScore(1)));
	//contractText->setPosition(SCORE_POSITION);

	static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyCardAboutToBePlayed>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyEndOfTrick>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyEndOfRound>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyNewRound>&>(*Application::getInstance()).registerObserver(*this);
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
		component->setOriginTopCenter(true);
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

		if (player_ptr->getPlayerIndex() % 2 == 0)
		{
			component->setRotation(90.f * (player_ptr->getPlayerIndex() == 0 ? 1 : -1));
		}

		component->setOriginCenter(true);
	}
}

sf::Vector2f GameState::calculateCardPosition(const Player& player, int cardIndex, int numCards) const
{
	//const float direction = PLAYER_POSITIONS[player.getPlayerIndex()].m_isReversedDirection ? -1 : 1;
	//float positionX = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.x + (cardOrder * CARD_WIDTH * (1 - CARD_OVERLAP_IN_HAND_PERCENT)) * direction;
	//if (PLAYER_POSITIONS[player.getPlayerIndex()].m_isHorizontallyCentered)
	//{
	//	positionX -= (player.getCards().size() / 2.f) * CARD_WIDTH * (1 - CARD_OVERLAP_IN_HAND_PERCENT);
	//}

	//float positionY = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.y;
	//return { positionX, positionY };

	if (PLAYER_POSITIONS[player.getPlayerIndex()].m_direction == PlayerPosition::Horizontal)
	{
		float positionX = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.x + (cardIndex * CARD_WIDTH * (1 - CARD_OVERLAP_IN_HAND_PERCENT));
		positionX -= (numCards / 2.f) * CARD_WIDTH * (1 - CARD_OVERLAP_IN_HAND_PERCENT); // center
		float positionY = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.y;
		return { positionX, positionY };
	}
	else
	{
		float positionX = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.x;
		float positionY = PLAYER_POSITIONS[player.getPlayerIndex()].m_position.y + (cardIndex * CARD_HEIGHT * (1 - CARD_OVERLAP_IN_HAND_PERCENT));
		positionY -= (numCards / 2.f) * CARD_HEIGHT * (1 - CARD_OVERLAP_IN_HAND_PERCENT); // center
		return { positionX, positionY };
	}
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
		UIManager::handleInputEvent(event);

		if (sf::Event::Closed == event.type)
		{
			window.close();
		}
		else if (sf::Event::KeyReleased == event.type && event.key.code == sf::Keyboard::Key::F5)
		{
			togglePause();
		}
	}
}

void GameState::update(float dtSeconds)
{
	UIManager::update(dtSeconds);

	if (m_delayGameSeconds > 0.f)
	{
		m_delayGameSeconds -= dtSeconds;
	}

	const bool shouldPause = m_delayGameSeconds > 0.f || m_shouldPauseGame || std::any_of(m_components.begin(), m_components.end(), [](auto& item) { return item->isMoving(); });
	if (!shouldPause)
	{
		m_belote.updateState();
	}
}

void GameState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::White);

	UIManager::draw(window);

	window.display();
}

void GameState::notify(const NotifyCardDealing& data)
{
	size_t index = 0;
	for (const Card* card : data.m_player.getCards())
	{
		const auto targetPosition = calculateCardPosition(data.m_player, index++, data.m_player.getCards().size());
		UIComponent* cardComponent = getComponent(card->toString());
		cardComponent->moveToPosition(targetPosition, CARD_DEALING_TIME_SECONDS);
		cardComponent->setVisible(true);

		if (card == &data.m_card)
		{
			renderOnTop(cardComponent);
		}
	}

	if (m_belote.getDeck().empty())
	{
		getComponent("card_back")->setVisible(false);
	}
}

void GameState::notify(const NotifyContractVote& data)
{
	UIComponent* component = getOrCreateComponent(std::format("bidding_{}", data.m_player.getPlayerIndex()));
	component->setVisible(true);
	component->setText(data.m_contract.toString());
	component->setPosition(PLAYER_BIDDING_TEXT_POSITIONS[data.m_player.getPlayerIndex()]);

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
		getComponent(card->toString())->moveToPosition(calculateCardPosition(data.m_player, index++, data.m_player.getCards().size() - 1), 0.4f);
	}

	delayGame(WAIT_TIME_AFTER_PLAYING);
}

void GameState::notify(const NotifyEndOfTrick& data)
{
	for (const Card* card : data.m_trick.getCards())
	{
		getComponent(card->toString())->setVisible(false);
	}
}

void GameState::notify(const NotifyEndOfRound& data)
{
	// Reset player names (cleaning contract voting)
	createPlayerNames();
	getOrCreateComponent("first_player")->setText(std::format("First player this round: {}", getPlayerName(*m_belote.getPlayers()[m_belote.getNextPlayerIndex(m_belote.getDealingPlayerIndex())])));

	UIComponent* scoreText = getOrCreateComponent("score");
	scoreText->setText(std::format("Score: {} - {}", m_belote.getTeamScore(0), m_belote.getTeamScore(1)));
}

void GameState::notify(const NotifyNewRound& data)
{
	for (size_t playerIndex = 0; playerIndex < 4; ++playerIndex)
	{
		getOrCreateComponent(std::format("bidding_{}", playerIndex))->setVisible(false);
	}
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