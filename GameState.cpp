#include "GameState.h"
#include "Application.h"

GameState::GameState(StateMachine& stateMachine)
	: BaseState(stateMachine)
{
	createCardSprites();

	const sf::Vector2u windowSize = Application::getInstance()->getWindow().getSize();
	m_deckPosition = { windowSize.x / 2.f, windowSize.y / 2.f };

	PlayerPosition player0{ {s_borderOffset, windowSize.y / 2.f}, false, false }; // left
	PlayerPosition player3{ {windowSize.x / 2.f, windowSize.y - s_borderOffset - m_cardSize.y}, true, false }; // bottom
	PlayerPosition player2{ {windowSize.x - s_borderOffset - m_cardSize.x, windowSize.y / 2.f}, false, true }; // right
	PlayerPosition player1{ {windowSize.x / 2.f, s_borderOffset}, true, false }; // up
	m_playerPositions = { player0, player3, player2, player1, };

	Application::getInstance()->getAssetsManager().loadTexture("assets/background.jpg", "assets/background.jpg");
	m_background.setTexture(Application::getInstance()->getAssetsManager().getTexture("assets/background.jpg"), true);

	for (const auto& player_ptr : m_belote.getPlayers())
	{
		TextData textData;
		const std::string playerText = std::format("Player {} ({})", player_ptr->getPlayerIndex(), player_ptr->isHuman() ? "human" : "AI");
		textData.m_text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());
		textData.m_text.setCharacterSize(36);
		textData.m_text.setFillColor(sf::Color::Black);
		textData.m_text.setString(playerText);
		sf::Vector2f position = this->calculateCardPosition(*player_ptr, 0);
		if (player_ptr->getPlayerIndex() == 3)
		{
			position.y += 20.f + m_cardSize.y;
		}
		else
		{
			position.y -= 20.f + textData.m_text.getGlobalBounds().height; // offset. TODO: move to a variable
		}
		textData.m_text.setPosition(position);
		textData.m_timeLeft = -1.f; // infinite
		m_texts[playerText] = std::make_unique<TextData>(std::move(textData));
	}

	static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).registerObserver(*this);
}

void GameState::createCardSprites()
{
	for (const Card* card : m_belote.getDeck())
	{
		const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(card->getRank()), stringFromSuit(card->getSuit()));
		Application::getInstance()->getAssetsManager().loadTexture(path, path);

		//m_cardSprites.emplace(card, Application::getInstance()->getAssetsManager().getTexture(path)); // CRASHING: todo - investigate
		const sf::Texture& texture = Application::getInstance()->getAssetsManager().getTexture(path);
		sf::Sprite sprite(texture);
		sprite.setScale(s_cardScale);
		m_cardSprites[card] = sprite;
	}

	m_cardSize = { m_cardSprites.begin()->second.getTexture()->getSize().x * s_cardScale.x, m_cardSprites.begin()->second.getTexture()->getSize().y * s_cardScale.y };

	m_cardBackground.setSize(m_cardSize);
	m_cardBackground.setFillColor(sf::Color::White);

	Application::getInstance()->getAssetsManager().loadTexture("assets/card_back.png", "assets/card_back.png");
	m_cardBack = sf::Sprite(Application::getInstance()->getAssetsManager().getTexture("assets/card_back.png"));
	m_cardBack.setScale(s_cardScale);
}

sf::Vector2f GameState::calculateCardPosition(const Player& player, int cardOrder) const
{
	const float direction = m_playerPositions[player.getPlayerIndex()].m_isReversedDirection ? -1 : 1;
	float positionX = m_playerPositions[player.getPlayerIndex()].m_position.x + (cardOrder * m_cardSize.x * 0.66) * direction;
	if (m_playerPositions[player.getPlayerIndex()].m_isHorizontallyCentered)
	{
		positionX -= (player.getCards().size() / 2.f) * m_cardSize.x * 0.66;
	}

	float positionY = m_playerPositions[player.getPlayerIndex()].m_position.y;
	return { positionX, positionY };
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
		return value->m_timeLeft == 0.f;
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
		m_cardBack.setPosition(m_deckPosition);
		window.draw(m_cardBack);
	}

	for (size_t i = 0; i < m_belote.getPlayers().size(); ++i)
	{
		for (size_t j = 0; j < m_belote.getPlayers()[i]->getCards().size(); ++j)
		{
			const Card* card = m_belote.getPlayers()[i]->getCards()[j];

			sf::Vector2f position;
			auto moveData = m_movingSprites.find(card);
			if (moveData != m_movingSprites.end())
			{
				const float elapsed = std::min(moveData->second.m_elapsedTimeSeconds / moveData->second.m_moveTime, 1.f);
				const float x = std::lerp(moveData->second.m_startPosition.x, moveData->second.m_endPosition.x, elapsed);
				const float y = std::lerp(moveData->second.m_startPosition.y, moveData->second.m_endPosition.y, elapsed);
				position = { x, y };
			}
			else
			{
				position = calculateCardPosition(*m_belote.getPlayers()[i], j);
			}

			sf::Sprite& sprite = m_cardSprites[card];
			sprite.setPosition(position);
			m_cardBackground.setPosition(position);

			window.draw(m_cardBackground);
			window.draw(sprite);
		}
	}

	for (auto& [key, textData] : m_texts)
	{
		window.draw(textData->m_text);
	}

	window.display();
}

void GameState::notify(const NotifyCardDealing& data)
{
	const auto targetPosition = calculateCardPosition(data.m_player, data.m_player.getCards().size() - 1);

	SpriteMoveData moveData;
	moveData.m_startPosition = m_deckPosition;
	moveData.m_endPosition = targetPosition;
	moveData.m_moveTime = 0.5f;
	moveData.m_sprite = &m_cardSprites[&data.m_card];
	m_movingSprites[&data.m_card] = moveData;

	m_belote.pauseStateMachine(true);
}

void GameState::notify(const NotifyContractVote& data)
{
	// easy hack for counter-clockwise order
	static constexpr int playerVotePosition[4] = { 0, 3, 2, 1, };

	const std::string key = std::format("player {} vote:", data.m_player.getPlayerIndex());
	if (!m_texts.contains(key)) // C++20
	{
		m_texts[key] = std::make_unique<TextData>();
		sf::Text& text = m_texts[key]->m_text;

		text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());
		text.setCharacterSize(36);
		text.setFillColor(sf::Color::Black);
		text.setString(key + contractToString(data.m_contract));
		sf::Vector2f position{ 0.f, text.getGlobalBounds().height * playerVotePosition[data.m_player.getPlayerIndex()] };
		text.setPosition(position);
		m_texts[key]->m_timeLeft = -1.f; // infinite
	}
	else
	{
		TextData& textData = *m_texts[key];
		sf::Text& text = textData.m_text;
		text.setString(text.getString() + ", " + contractToString(data.m_contract));
		textData.m_timeLeft = -1.f; // infinite
	}

	delayGame(1.f); // TODO: MOVE into variable
}

void GameState::delayGame(float seconds)
{
	m_delayGameSeconds = seconds;
}

void GameState::togglePause()
{
	m_shouldPauseGame = !m_shouldPauseGame;

	if (!m_shouldPauseGame)
	{
		m_texts.erase("pause"); // TODO: I don't like this being deleted and created every time. TextData should be reworked
		return;
	}

	TextData textData;
	textData.m_text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());
	textData.m_text.setCharacterSize(72);
	textData.m_text.setFillColor(sf::Color::Red);
	textData.m_text.setString("GAME PAUSED!");
	textData.m_text.setPosition(m_deckPosition);
	//center
	textData.m_text.setOrigin({ textData.m_text.getGlobalBounds().width / 2.f + textData.m_text.getLocalBounds().left, textData.m_text.getGlobalBounds().height / 2.f + textData.m_text.getLocalBounds().top });
	textData.m_timeLeft = -1.f; // infinite
	m_texts["pause"] = std::make_unique<TextData>(std::move(textData));
}
