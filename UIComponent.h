#pragma once

#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class UIComponent : public sf::Drawable
{
public:
	UIComponent() = default;

	void					addSprite(const std::string& texturePath, float scale = 1.f);

	std::string				getText() const { return m_text.getString().toAnsiString(); }
	void					setText(const std::string& str, const sf::Color& color = sf::Color::Black, unsigned int size = 36u, bool centered = false);
	void					setBackground(const sf::Color& color);

	void					setPosition(const sf::Vector2f& position);

	virtual void			draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	void					updateBackgroundRectangle();

	std::unique_ptr<sf::RectangleShape> m_background;
	std::vector<sf::Sprite> m_sprites;
	sf::Text				m_text;

	sf::Vector2f			m_position;

	// TODO: animation, visibility, markForDestroy
};

