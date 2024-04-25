#pragma once

#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class UIComponent : public sf::Drawable
{
public:
	UIComponent(const std::string& key);

	void								addSprite(const std::string& texturePath, float scale = 1.f);

	std::string							getText() const { return m_text.getString().toAnsiString(); }
	void								setText(const std::string& str, const sf::Color& color = sf::Color::Black, unsigned int size = 36u, bool centered = false);
	void								setBackground(const sf::Color& color);

	void								setPosition(const sf::Vector2f& position);
	void								moveToPosition(const sf::Vector2f& targetPosition, float animationTime = 0.5f);
	bool								isMoving() const { return m_moveAnimation != nullptr; }

	void								onUpdate(float deltaTimeSeconds);

	virtual void						draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void								setVisible(bool visible) { m_isVisible = visible; }
	bool								IsVisible() const { return m_isVisible; }

	const std::string&					getKey() const { return m_key; }

private:
	struct MoveAnimation
	{
		sf::Vector2f m_startPosition;
		sf::Vector2f m_endPosition;
		float m_animationTimeSeconds;
		float m_elapsedTimeSeconds = 0.f;

		MoveAnimation(const sf::Vector2f& startPosition, const sf::Vector2f& endPosition, float animationTimeSeconds = 0.5f) 
			: m_startPosition(startPosition)
			, m_endPosition(endPosition)
			, m_animationTimeSeconds(animationTimeSeconds)
		{}
	};

	void								updateBackgroundRectangle();

	std::string							m_key;

	std::unique_ptr<sf::RectangleShape> m_background;
	std::vector<sf::Sprite>				m_sprites;
	sf::Text							m_text;

	sf::Vector2f						m_position;

	std::unique_ptr<MoveAnimation>		m_moveAnimation;

	bool								m_isVisible = true;

	// TODO: markForDestroy
};

