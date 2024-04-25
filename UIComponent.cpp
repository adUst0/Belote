#include "UIComponent.h"
#include "Application.h"

UIComponent::UIComponent(const std::string& key)
	: m_key(key)
{

}

void UIComponent::addSprite(const std::string& texturePath, float scale)
{
	Application::getInstance()->getAssetsManager().loadTexture(texturePath, texturePath);
	const sf::Texture& texture = Application::getInstance()->getAssetsManager().getTexture(texturePath);

	sf::Sprite& sprite = m_sprites.emplace_back(texture);
	sprite.setScale({ scale, scale });

	updateBackgroundRectangle();
}

void UIComponent::setText(const std::string& str, const sf::Color& color /*= sf::Color::Black*/, unsigned int size /*= 36u*/, bool centered /*= false*/)
{
	m_text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());

	m_text.setCharacterSize(size);
	m_text.setFillColor(color);
	m_text.setString(str);

	if (centered)
	{
		m_text.setOrigin({ m_text.getGlobalBounds().width / 2.f + m_text.getLocalBounds().left, m_text.getGlobalBounds().height / 2.f + m_text.getLocalBounds().top });
	}

	updateBackgroundRectangle();
}

void UIComponent::setBackground(const sf::Color& color)
{
	if (!m_background)
	{
		m_background = std::make_unique<sf::RectangleShape>();
	}

	m_background->setFillColor(color);
	updateBackgroundRectangle();
}

void UIComponent::setPosition(const sf::Vector2f& position)
{
	m_position = position;

	if (m_background)
	{
		m_background->setPosition(position);
	}

	m_text.setPosition(position);

	for (sf::Sprite& sprite : m_sprites)
	{
		sprite.setPosition(position);
	}
}

void UIComponent::moveToPosition(const sf::Vector2f& targetPosition, float animationTime /*= 0.5f*/)
{
	m_moveAnimation = std::make_unique<MoveAnimation>(m_position, targetPosition, animationTime);
}

void UIComponent::onUpdate(float deltaTimeSeconds)
{
	if (m_moveAnimation)
	{
		m_moveAnimation->m_elapsedTimeSeconds += deltaTimeSeconds;

		const float elapsed = std::min(m_moveAnimation->m_elapsedTimeSeconds / m_moveAnimation->m_animationTimeSeconds, 1.f);
		const float x = std::lerp(m_moveAnimation->m_startPosition.x, m_moveAnimation->m_endPosition.x, elapsed);
		const float y = std::lerp(m_moveAnimation->m_startPosition.y, m_moveAnimation->m_endPosition.y, elapsed);
		setPosition({ x, y });

		if (elapsed == 1.f)
		{
			m_moveAnimation.reset();
		}
	}
}

void UIComponent::draw(sf::RenderTarget& target, sf::RenderStates /*states*/) const
{
	if (!m_isVisible || m_text.getString().isEmpty() && m_sprites.empty())
	{
		return;
	}

	if (m_background)
	{
		target.draw(*m_background);
	}

	for (const sf::Sprite& sprite : m_sprites)
	{
		target.draw(sprite);
	}

	if (!m_text.getString().isEmpty())
	{
		target.draw(m_text);
	}
}

void UIComponent::updateBackgroundRectangle()
{
	if (!m_background)
	{
		return;
	}

	if (!m_sprites.empty())
	{
		m_background->setSize({ m_sprites[0].getGlobalBounds().width, m_sprites[0].getGlobalBounds().height });
	}
	else if (!m_text.getString().isEmpty())
	{
		m_background->setSize({ m_text.getGlobalBounds().width, m_text.getGlobalBounds().height });
	}
}