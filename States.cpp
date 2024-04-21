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
	StateBase::handleInput();

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

	m_belote.update();
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

void TestBeloteState::update(float /*dtSeconds*/)
{
	m_belote.update();
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

