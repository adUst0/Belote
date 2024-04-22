#include "States.h"

#include <SFML/Graphics.hpp>
#include "Application.h"
#include <array>
#include <format>

std::array<Rank, 8> beloteCards
{
	Rank::Seven,
	Rank::Eight,
	Rank::Nine,
	Rank::Ten,
	Rank::Jack,
	Rank::Queen,
	Rank::King,
	Rank::Ace
};

void TestRenderCardsState::onEnterState(bool isResume)
{
	if (isResume)
	{
		return;
	}

	createDeck();
}

void TestRenderCardsState::handleInput()
{
	BaseState::handleInput();

	for (auto& [card, sprite] : m_sprites)
	{
		if (Application::getInstance()->getInputManager().isSpriteClicked(sprite, sf::Mouse::Button::Left, Application::getInstance()->getWindow()))
		{
			SpriteMoveData moveData;
			moveData.m_startPosition = sprite.getPosition();
			moveData.m_endPosition = sf::Vector2f(640, 360);
			moveData.m_sprite = &sprite;
			m_movingSprites[card] = moveData;
		}
	}
}

void TestRenderCardsState::update(float dtSeconds)
{
	const float moveTime = 2.f;

	for (auto it = m_movingSprites.begin(); it != m_movingSprites.end();)
	{
		const float elapsed = std::min((it->second.m_elapsedTimeSeconds + dtSeconds) / moveTime, 1.f);
		const float x = std::lerp(it->second.m_startPosition.x, it->second.m_endPosition.x, elapsed);
		const float y = std::lerp(it->second.m_startPosition.y, it->second.m_endPosition.y, elapsed);
		it->second.m_sprite->setPosition(sf::Vector2f(x, y));
		it->second.m_elapsedTimeSeconds += elapsed;

		// put card to be rendered on top. TODO: that has to be some UI list, not the real deck
		auto card = std::find_if(m_deck.begin(), m_deck.end(), [this, it](const auto& ptr) { return ptr.get() == it->first; });
		if (card != m_deck.end())
		{
			std::rotate(card, card + 1, m_deck.end());
		}

		if (elapsed == 1.f)
		{
			it = m_movingSprites.erase(it);
		}
		else
		{
			++it;
		}
	}

	m_belote.updateState();
}

void TestRenderCardsState::createDeck()
{
	const float offset = 5.f;

	m_deck.clear();

	for (int i = 0; i < (int8_t)Suit::Num; ++i)
	{
		int colIndex = 0;
		for (Rank rank : beloteCards)
		{
			auto& card_ptr = m_deck.emplace_back(std::make_unique<Card>(Suit(i), rank));

			const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(rank), stringFromSuit(Suit(i)));
			Application::getInstance()->getAssetsManager().loadTexture(path, path);

			m_sprites.emplace(card_ptr.get(), Application::getInstance()->getAssetsManager().getTexture(path));
			sf::Sprite& sprite = m_sprites[card_ptr.get()];
			sprite.setScale(s_cardScale);

			const sf::Vector2f size = getCardSize();
			const sf::Vector2f position{ size.x * colIndex + offset * (colIndex + 1), size.y * i + offset * (i + 1) };
			sprite.setPosition(position);
			++colIndex;
		}
	}

	m_cardBackground.setSize(getCardSize());
	m_cardBackground.setFillColor(sf::Color::White);
}

void TestRenderCardsState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::Green);

	for (const auto& card_ptr : m_deck)
	{
		sf::Sprite& sprite = m_sprites[card_ptr.get()];
		m_cardBackground.setPosition(sprite.getPosition());
		
		window.draw(m_cardBackground);
		window.draw(sprite);
	}

	sf::CircleShape circle(5.f);
	circle.setPosition(sf::Vector2f(640, 360));
	circle.setFillColor(sf::Color::Red);
	window.draw(circle);

	window.display();
}

sf::Vector2f TestRenderCardsState::getCardSize() const
{
	const sf::Sprite& sprite = m_sprites.at(m_deck[0].get());
	return { sprite.getTexture()->getSize().x * s_cardScale.x, sprite.getTexture()->getSize().y * s_cardScale.y };
}

//////////////////////////////////////////////////////////////////////////////////

void TestBeloteState::handleInput()
{

}

void TestBeloteState::update(float dtSeconds)
{
	m_belote.updateState();
}

void TestBeloteState::draw()
{
	Application* application = Application::getInstance();
	application->getWindow().clear(sf::Color::Green);

	const float cardOffset = 10.f;

	const float cardSizeX = 111.f; // TODO: hardcoded
	const float cardSizeY = 161.f;
	sf::Vector2f PlayerLocation[4] = {
		{0.f, 450.f},
		{800.f - (4 * cardSizeX + cardOffset), 20.f},
		{1600.f - (8 * cardSizeX + cardOffset), 450.f},
		{800.f - (4 * cardSizeX + cardOffset), 900.f - cardSizeY}
	};

	for (int i = 0; i < m_belote.getPlayers().size(); ++i)
	{
		const Player& player = *m_belote.getPlayers()[i];

		int cardIndex = 0;
		for (int j = 0; j < player.getCards().size(); ++j)
		{
			const Card& card = *player.getCards()[j];

			const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(card.getRank()), stringFromSuit(card.getSuit()));
			if (!application->getAssetsManager().hasTexture(path))
			{
				application->getAssetsManager().loadTexture(path, path);
			}
			
			sf::Sprite sprite(application->getAssetsManager().getTexture(path));
			sprite.setScale(s_cardScale);

			const sf::Vector2f size = { sprite.getTexture()->getSize().x * sprite.getScale().x, sprite.getTexture()->getSize().y * sprite.getScale().y };
			float positionX = PlayerLocation[i].x + (j * size.x*0.66 + cardOffset);
			float positionY = PlayerLocation[i].y - cardOffset;

			m_cardBackground.setSize(size);
			m_cardBackground.setFillColor(sf::Color::White);
			m_cardBackground.setPosition({ positionX, positionY });
;
			sprite.setPosition({ positionX, positionY });

			application->getWindow().draw(m_cardBackground);
			application->getWindow().draw(sprite);

			++cardIndex;
		}
	}


	application->getWindow().display();
}

//////////////////////////////////////////////////////////////////////////////////

GameState::GameState(StateMachine& stateMachine)
	: BaseState(stateMachine)
{
	createCardSprites();

	const sf::Vector2u windowSize = Application::getInstance()->getWindow().getSize();
	m_deckPosition = { windowSize.x / 2.f, windowSize.y / 2.f };

	PlayerPosition player0{ {s_borderOffset, windowSize.y / 2.f}, false, false }; // left
	PlayerPosition player1{ {windowSize.x / 2.f, windowSize.y - s_borderOffset - m_cardSize.y}, true, false }; // bottom
	PlayerPosition player2{ {windowSize.x - s_borderOffset - m_cardSize.x, windowSize.y / 2.f}, false, true }; // right
	PlayerPosition player3{ {windowSize.x / 2.f, s_borderOffset}, true, false }; // up
	m_playerPositions = { player0, player1, player2, player3 };
}

void GameState::createCardSprites()
{
	for (const Card* card : m_belote.getDeck())
	{
		const std::string path = std::format("assets/{}_of_{}.png", stringFromRank(card->getRank()), stringFromSuit(card->getSuit()));
		Application::getInstance()->getAssetsManager().loadTexture(path, path);

		m_cardSprites.emplace(card, Application::getInstance()->getAssetsManager().getTexture(path));
		sf::Sprite& sprite = m_cardSprites[card];
		sprite.setScale(s_cardScale);
	}

	m_cardSize = { m_cardSprites.begin()->second.getTexture()->getSize().x * s_cardScale.x, m_cardSprites.begin()->second.getTexture()->getSize().y * s_cardScale.y };

	m_cardBackground.setSize(m_cardSize);
	m_cardBackground.setFillColor(sf::Color::White);
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
	BaseState::handleInput();

	// TODO:
	//for (auto& [card, sprite] : m_sprites)
	//{
	//	if (Application::getInstance()->getInputManager().isSpriteClicked(sprite, sf::Mouse::Button::Left, Application::getInstance()->getWindow()))
	//	{
	//		SpriteMoveData moveData;
	//		moveData.m_startPosition = sprite.getPosition();
	//		moveData.m_endPosition = sf::Vector2f(640, 360);
	//		moveData.m_sprite = &sprite;
	//		m_movingSprites[card] = moveData;
	//	}
	//}
}

void GameState::update(float dtSeconds)
{
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

	m_belote.updateState();
}

void GameState::draw()
{
	sf::RenderWindow& window = Application::getInstance()->getWindow();
	window.clear(sf::Color::Green);

	if (!m_belote.getDeck().empty())
	{
		m_cardBackground.setPosition(m_deckPosition);
		window.draw(m_cardBackground);
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

	window.display();
}

void GameState::notifyCardDealing(const Player& player, const Card& card)
{
	const auto targetPosition = calculateCardPosition(player, player.getCards().size() - 1);
	
	SpriteMoveData data;
	data.m_startPosition = m_deckPosition;
	data.m_endPosition = targetPosition;
	data.m_moveTime = 1.f;
	data.m_sprite = &m_cardSprites[&card];
	m_movingSprites[&card] = data;
}
