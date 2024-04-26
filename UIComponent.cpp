#include "UIComponent.h"
#include "Application.h"

UIComponent::UIComponent(const std::string& key)
	: m_key(key)
{

}

void UIComponent::addSprite(const std::string& texturePath, const sf::Vector2f& scale)
{
	Application::getInstance()->getAssetsManager().loadTexture(texturePath, texturePath);
	const sf::Texture& texture = Application::getInstance()->getAssetsManager().getTexture(texturePath);

	sf::Sprite& sprite = m_sprites.emplace_back(texture);
	sprite.setScale(scale);

	updateBackgroundRectangleSize();
	updateOrigin();
}

void UIComponent::setText(const std::string& str, const sf::Color& color /*= sf::Color::Black*/, unsigned int size /*= 36u*/)
{
	m_text.setFont(Application::getInstance()->getAssetsManager().getDefaultFont());

	m_text.setCharacterSize(size);
	m_text.setFillColor(color);
	m_text.setString(str);

	updateBackgroundRectangleSize();
	updateOrigin();
}

void UIComponent::setBackground(const sf::Color& color)
{
	if (!m_background)
	{
		m_background = std::make_unique<sf::RectangleShape>();
	}

	m_background->setFillColor(color);
	updateBackgroundRectangleSize();
	updateOrigin();
}

void UIComponent::setOriginCenter(bool value)
{
	m_isOriginCenter = value;
	updateOrigin();
}

void UIComponent::setPosition(const sf::Vector2f& position, bool cancelCurrentMoveAnimation /*= true*/)
{
	if (cancelCurrentMoveAnimation && m_moveAnimation)
	{
		m_moveAnimation.reset();
	}

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

void UIComponent::onMouseLeftClick(const std::function<void(void)>& callback)
{
	m_onMouseLeftClick = callback;
}

void UIComponent::onMouseOn(const std::function<void(void)>& callback)
{
	m_onMouseOn = callback;
}

void UIComponent::onMouseOff(const std::function<void(void)>& callback)
{
	m_onMouseOff = callback;
}

bool UIComponent::handleLeftMouseClick(const sf::Vector2f& mousePosition)
{
	if (!m_isVisible || !m_onMouseLeftClick || m_sprites.empty() && m_text.getString().isEmpty())
	{
		return false;
	}

	if (isMouseOver(mousePosition))
	{
		m_onMouseLeftClick();
		return true;
	}

	return false;
}

bool UIComponent::handleMouseOver(const sf::Vector2f& mousePosition)
{
	const bool wasMouseOver = m_isMouseOver;
	m_isMouseOver = isMouseOver(mousePosition);

	if (!wasMouseOver && m_isMouseOver)
	{
		if (m_onMouseOn)
		{
			m_onMouseOn();
		}
	}
	else if (wasMouseOver && !m_isMouseOver)
	{
		if (m_onMouseOff)
		{
			m_onMouseOff();
		}
	}

	return m_isMouseOver;
}

void UIComponent::onUpdate(float deltaTimeSeconds)
{
	if (m_moveAnimation)
	{
		m_moveAnimation->m_elapsedTimeSeconds += deltaTimeSeconds;

		const float elapsed = std::min(m_moveAnimation->m_elapsedTimeSeconds / m_moveAnimation->m_animationTimeSeconds, 1.f);
		const float x = std::lerp(m_moveAnimation->m_startPosition.x, m_moveAnimation->m_endPosition.x, elapsed);
		const float y = std::lerp(m_moveAnimation->m_startPosition.y, m_moveAnimation->m_endPosition.y, elapsed);
		setPosition({ x, y }, false);

		if (elapsed == 1.f)
		{
			m_moveAnimation.reset();
		}
	}
}

void UIComponent::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (!m_isVisible || m_text.getString().isEmpty() && m_sprites.empty())
	{
		return;
	}

	if (m_isMouseOver && m_hoverState)
	{
		m_hoverState->draw(target, states);
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

bool UIComponent::isMouseOver(const sf::Vector2f& mousePosition) const
{
	const sf::FloatRect rectangle = m_sprites.empty() ? m_text.getGlobalBounds() : m_sprites[0].getGlobalBounds();
	return rectangle.contains(mousePosition);
}

void UIComponent::setHoverState(std::unique_ptr<UIComponent>&& state)
{
	m_hoverState = std::move(state);
}

void UIComponent::updateBackgroundRectangleSize()
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

void UIComponent::updateOrigin()
{
	if (m_isOriginCenter)
	{
		const sf::FloatRect textBounds = m_text.getLocalBounds();
		m_text.setOrigin({ textBounds.width / 2.f, textBounds.height / 2.f + textBounds.top });
	
		for (sf::Sprite& sprite : m_sprites)
		{
			const sf::FloatRect spriteBounds = sprite.getLocalBounds();
			sprite.setOrigin({ spriteBounds.width / 2.f, spriteBounds.height / 2.f });
		}

		if (m_background)
		{
			const sf::FloatRect bgBounds = m_background->getLocalBounds();
			m_background->setOrigin({ bgBounds.width / 2.f + bgBounds.left, bgBounds.height / 2.f + bgBounds.top });
		}
	}
}
