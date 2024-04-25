#pragma once

#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

class UIComponent : public sf::Drawable
{
public:
	UIComponent(const std::string& key);

	void								addSprite(const std::string& texturePath, float scale = 1.f);

	std::string							getText() const { return m_text.getString().toAnsiString(); }
	void								setText(const std::string& str, const sf::Color& color = sf::Color::Black, unsigned int size = 36u);
	void								setBackground(const sf::Color& color);

	void								setOriginCenter(bool value);

	void								setPosition(const sf::Vector2f& position, bool cancelCurrentMoveAnimation = true);
	void								moveToPosition(const sf::Vector2f& targetPosition, float animationTime = 0.5f);
	bool								isMoving() const { return m_moveAnimation != nullptr; }

	void								setVisible(bool visible) { m_isVisible = visible; }
	bool								IsVisible() const { return m_isVisible; }

	const std::string&					getKey() const { return m_key; }

	void								onMouseLeftClick(const std::function<void(void)>& callback);

	// Internal methods to be called only by the Class that manages the UI components
	bool								handleLeftMouseClick(const sf::Vector2f& mousePosition);
	void								onUpdate(float deltaTimeSeconds);
	virtual void						draw(sf::RenderTarget& target, sf::RenderStates states) const override;

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

	void								updateBackgroundRectangleSize();
	void								updateOrigin();

	std::string							m_key;

	std::unique_ptr<sf::RectangleShape> m_background;
	std::vector<sf::Sprite>				m_sprites;
	sf::Text							m_text;

	sf::Vector2f						m_position;

	std::unique_ptr<MoveAnimation>		m_moveAnimation;

	bool								m_isVisible = true;
	bool								m_isOriginCenter = false;

	std::function<void(void)>			m_onMouseLeftClick;

	// TODO: Origin center for all components, markForDestroy, hover states or callback
};

