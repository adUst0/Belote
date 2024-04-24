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

	static const sf::Vector2f			CARD_POSITION_ON_TABLE[4] { // Current trick cards
		{SCREEN_CENTER.x - CARD_WIDTH, SCREEN_CENTER.y}, // left
		{SCREEN_CENTER.x, SCREEN_CENTER.y + CARD_HEIGHT}, // top
		{SCREEN_CENTER.x + CARD_WIDTH, SCREEN_CENTER.y}, // right
		{SCREEN_CENTER.x, SCREEN_CENTER.y - CARD_HEIGHT}, // bottom
	};


}

GameState::GameState(StateMachine& stateMachine)
	: BaseState(stateMachine)
{
	createCardSprites();

	Application::getInstance()->getAssetsManager().loadTexture("assets/background.jpg", "assets/background.jpg");
	m_background.setTexture(Application::getInstance()->getAssetsManager().getTexture("assets/background.jpg"), true);

	for (const auto& player_ptr : m_belote.getPlayers())
	{
	//	TextData textData;
	//	const std::string playerText = std::format("Player {} ({})", player_ptr->getPlayerIndex(), player_ptr->isHuman() ? "human" : "AI");
	//	textData.m_text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());
	//	textData.m_text.setCharacterSize(36);
	//	textData.m_text.setFillColor(sf::Color::Black);
	//	textData.m_text.setString(playerText);
	//	textData.m_text.setPosition(PLAYER_NAME_POSITIONS[player_ptr->getPlayerIndex()]);
	//	textData.m_timeLeft = -1.f; // infinite
	//	m_texts[playerText] = std::make_unique<TextData>(std::move(textData));
		std::unique_ptr<UIComponent>& component = m_uiComponents[std::format("player_{}", player_ptr->getPlayerIndex())];
		component = std::make_unique<UIComponent>();
		component->setText(getPlayerName(*player_ptr));
		component->setPosition(PLAYER_NAME_POSITIONS[player_ptr->getPlayerIndex()]);
	}
	

	static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyCardAboutToBePlayed>&>(*Application::getInstance()).registerObserver(*this);
}

void GameState::createCardSprites()
{
	for (const Card* card : m_belote.getDeck())
	{
		const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(card->getRank()), stringFromSuit(card->getSuit()));
		Application::getInstance()->getAssetsManager().loadTexture(path, path);

		const sf::Texture& texture = Application::getInstance()->getAssetsManager().getTexture(path);
		sf::Sprite sprite(texture);
		sprite.setScale(CARD_SCALE);
		m_cardSprites[card] = sprite;
	}

	m_cardBackground.setSize({ CARD_WIDTH, CARD_HEIGHT });
	m_cardBackground.setFillColor(sf::Color::White);

	Application::getInstance()->getAssetsManager().loadTexture("assets/card_back.png", "assets/card_back.png");
	m_cardBack = sf::Sprite(Application::getInstance()->getAssetsManager().getTexture("assets/card_back.png"));
	m_cardBack.setScale(CARD_SCALE);
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
	return CARD_POSITION_ON_TABLE[player.getPlayerIndex()];
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
}

void GameState::update(float dtSeconds)
{
	// Update moving sprites
	for (auto& [card, data] : m_movingSprites)
	{
		data.m_elapsedTimeSeconds += dtSeconds;
	}
	// C++20 required
	std::erase_if(m_movingSprites, [](const auto& item)
	{
		auto const& [key, value] = item;
		return value.m_elapsedTimeSeconds >= value.m_moveTime;
	});

	// Update texts
	for (auto& [key, textData] : m_texts)
	{
		if (textData->m_timeLeft > 0.f)
		{
			textData->m_timeLeft = std::max(0.f, textData->m_timeLeft - dtSeconds);
		}
	}
	// C++20 required
	std::erase_if(m_texts, [](const auto& item)
	{
		auto const& [key, value] = item;
		return value->m_timeLeft == 0.f && value->m_deleteOnExpiration;
	});

	if (m_delayGameSeconds > 0.f)
	{
		m_delayGameSeconds -= dtSeconds;
	}

	const bool shouldPause = !m_movingSprites.empty() || m_delayGameSeconds > 0.f || m_shouldPauseGame;
	m_belote.pauseStateMachine(shouldPause);

	m_belote.updateState();
}

void GameState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::Green);

	window.draw(m_background);

	if (!m_belote.getDeck().empty())
	{
		m_cardBack.setPosition(DECK_POSITION);
		window.draw(m_cardBack);
	}

	for (size_t i = 0; i < m_belote.getPlayers().size(); ++i)
	{
		for (size_t j = 0; j < m_belote.getPlayers()[i]->getCards().size(); ++j)
		{
			const Card* card = m_belote.getPlayers()[i]->getCards()[j];

			if (m_movingSprites.find(card) != m_movingSprites.end())
			{
				continue;
				
			}

			sf::Vector2f position = calculateCardPosition(*m_belote.getPlayers()[i], j);

			sf::Sprite& sprite = m_cardSprites[card];
			sprite.setPosition(position);
			m_cardBackground.setPosition(position);

			window.draw(m_cardBackground);
			window.draw(sprite);
		}
	}

	for (int i = m_belote.getCurrentTrickCards().size() - 1; i >= 0; --i)
	{
		const Card* card = m_belote.getCurrentTrickCards()[i];
		if (m_movingSprites.find(card) != m_movingSprites.end())
		{
			continue;
		}

		size_t playerIndex = m_belote.getActivePlayerIndex();
		for (size_t j = i; j < m_belote.getCurrentTrickCards().size() - 1; ++j)
		{
			playerIndex = m_belote.getPreviousPlayerIndex(playerIndex);
		}

		const sf::Vector2f position = getCardPositionOnTable(*m_belote.getPlayers()[playerIndex]);
		sf::Sprite& sprite = m_cardSprites[card];
		sprite.setPosition(position);
		m_cardBackground.setPosition(position);

		window.draw(m_cardBackground);
		window.draw(sprite);
	}

	for (auto& [card, moveData] : m_movingSprites)
	{
		const float elapsed = std::min(moveData.m_elapsedTimeSeconds / moveData.m_moveTime, 1.f);
		const float x = std::lerp(moveData.m_startPosition.x, moveData.m_endPosition.x, elapsed);
		const float y = std::lerp(moveData.m_startPosition.y, moveData.m_endPosition.y, elapsed);
		sf::Vector2f position{ x, y };

		moveData.m_sprite->setPosition(position);
		m_cardBackground.setPosition(position);

		window.draw(m_cardBackground);
		window.draw(*moveData.m_sprite);
	}

	for (auto& [key, textData] : m_texts)
	{
		if (textData->m_timeLeft != 0.f)
		{
			window.draw(textData->m_text);
		}
	}

	for (auto& [id, component_ptr] : m_uiComponents)
	{
		window.draw(*component_ptr);
	}

	window.display();
}

void GameState::notify(const NotifyCardDealing& data)
{
	const auto targetPosition = calculateCardPosition(data.m_player, data.m_player.getCards().size() - 1);

	SpriteMoveData moveData;
	moveData.m_startPosition = DECK_POSITION;
	moveData.m_endPosition = targetPosition;
	moveData.m_moveTime = 0.5f;
	moveData.m_sprite = &m_cardSprites[&data.m_card];
	m_movingSprites[&data.m_card] = moveData;

	m_belote.pauseStateMachine(true);
}

void GameState::notify(const NotifyContractVote& data)
{
	const std::string key = std::format("player {} vote:", data.m_player.getPlayerIndex());
	if (!m_uiComponents.contains(key)) // C++20
	{
		std::unique_ptr<UIComponent>& component = m_uiComponents[key];
		component = std::make_unique<UIComponent>();
		component->setText(key + contractToString(data.m_contract));
		component->setPosition({ 0.f, 36.f * data.m_player.getPlayerIndex() });
	}
	else
	{
		m_uiComponents[key]->setText(m_uiComponents[key]->getText() + ", " + contractToString(data.m_contract));
	}

	delayGame(WAIT_TIME_AFTER_BIDDING);
}

void GameState::notify(const NotifyCardAboutToBePlayed& data)
{
	sf::Sprite& sprite = m_cardSprites[&data.m_card];

	SpriteMoveData moveData;
	moveData.m_startPosition = sprite.getPosition();
	moveData.m_endPosition = getCardPositionOnTable(data.m_player);
	moveData.m_moveTime = 0.5f;
	moveData.m_sprite = &sprite;
	m_movingSprites[&data.m_card] = moveData;

	m_belote.pauseStateMachine(true);
	delayGame(WAIT_TIME_AFTER_PLAYING);
}

void GameState::delayGame(float seconds)
{
	m_delayGameSeconds = seconds;
}

void GameState::togglePause()
{
	m_shouldPauseGame = !m_shouldPauseGame;

	if (!m_texts.contains("pause")) // C++20
	{
		TextData textData;
		textData.m_text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());
		textData.m_text.setCharacterSize(72);
		textData.m_text.setFillColor(sf::Color::Red);
		textData.m_text.setString("GAME PAUSED!");
		textData.m_text.setPosition(DECK_POSITION );
		//center
		textData.m_text.setOrigin({ textData.m_text.getGlobalBounds().width / 2.f + textData.m_text.getLocalBounds().left, textData.m_text.getGlobalBounds().height / 2.f + textData.m_text.getLocalBounds().top });
		m_texts["pause"] = std::make_unique<TextData>(std::move(textData));
	}

	m_texts["pause"]->m_timeLeft = m_shouldPauseGame ? -1.f : 0.f;
}