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
	: BaseState(stateMachine)
{
	auto& background = m_uiComponents.emplace_back(std::make_unique<UIComponent>("background"));
	background->addSprite("assets/background.jpg");

	createCardSprites();
	createPlayerNames();

	auto& pauseText = m_uiComponents.emplace_back(std::make_unique<UIComponent>("pause"));
	pauseText->setText("GAME PAUSED!", sf::Color::Red, 72, true);
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
		std::unique_ptr<UIComponent>& component = m_uiComponents.emplace_back(std::make_unique<UIComponent>(card->toString()));

		const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(card->getRank()), stringFromSuit(card->getSuit()));
		component->addSprite(path, CARD_SCALE.x);
		component->setBackground(sf::Color::White);
		component->setPosition(DECK_POSITION);
		component->setVisible(false);

		Application::getInstance()->getAssetsManager().loadTexture(path, path);
	}

	std::unique_ptr<UIComponent>& card_back = m_uiComponents.emplace_back(std::make_unique<UIComponent>("card_back"));
	card_back->addSprite("assets/card_back.png", CARD_SCALE.x);
	card_back->setPosition(DECK_POSITION);
}

void GameState::createPlayerNames()
{
	for (const auto& player_ptr : m_belote.getPlayers())
	{
		std::unique_ptr<UIComponent>& component = m_uiComponents.emplace_back(std::make_unique<UIComponent>(std::format("player_{}", player_ptr->getPlayerIndex())));
		component->setText(getPlayerName(*player_ptr));
		component->setPosition(PLAYER_NAME_POSITIONS[player_ptr->getPlayerIndex()]);
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
		else if (sf::Event::KeyReleased == event.type && event.key.code == sf::Keyboard::Key::F5)
		{
			togglePause();
		}
	}

	// TODO: UIComponent Click
}

void GameState::update(float dtSeconds)
{
	for (auto& component : m_uiComponents)
	{
		component->onUpdate(dtSeconds);
	}

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
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::Green);

	for (auto& component_ptr : m_uiComponents)
	{
		window.draw(*component_ptr);
	}

	window.display();
}

void GameState::notify(const NotifyCardDealing& data)
{
	const auto targetPosition = calculateCardPosition(data.m_player, data.m_player.getCards().size() - 1);

	UIComponent* card = findComponent(data.m_card.toString());
	card->moveToPosition(targetPosition, 1.f);
	card->setVisible(true);

	if (m_belote.getDeck().empty())
	{
		findComponent("card_back")->setVisible(false);
	}
}

void GameState::notify(const NotifyContractVote& data)
{
	const std::string key = std::format("player {} vote:", data.m_player.getPlayerIndex());
	if (!findComponent(key))
	{
		std::unique_ptr<UIComponent>& component = m_uiComponents.emplace_back(std::make_unique<UIComponent>(key));
		component->setText(key + contractToString(data.m_contract));
		component->setPosition({ 0.f, 36.f * data.m_player.getPlayerIndex() });
	}
	else
	{
		findComponent(key)->setText(findComponent(key)->getText() + ", " + contractToString(data.m_contract));
	}

	delayGame(WAIT_TIME_AFTER_BIDDING);
}

void GameState::notify(const NotifyCardAboutToBePlayed& data)
{
	findComponent(data.m_card.toString())->moveToPosition(getCardPositionOnTable(data.m_player));

	delayGame(WAIT_TIME_AFTER_PLAYING);

	// TODO: update card UI positions
}

void GameState::delayGame(float seconds)
{
	m_delayGameSeconds = seconds;
}

void GameState::togglePause()
{
	m_shouldPauseGame = !m_shouldPauseGame;

	findComponent("pause")->setVisible(m_shouldPauseGame);
}

UIComponent* GameState::findComponent(const std::string& key)
{
	auto iter = std::find_if(m_uiComponents.begin(), m_uiComponents.end(), [&key](auto& component)
	{
		return component->getKey() == key;
	});

	return iter == m_uiComponents.end() ? nullptr : (*iter).get();
}