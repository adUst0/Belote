#pragma once

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <string>

class AssetManager
{
public:
	AssetManager();

	void							loadTexture(const std::string& id, const std::string& fileName);
	const sf::Texture&				getTexture(const std::string& id) const;
	bool							hasTexture(const std::string& id) const;

	void							loadFont(const std::string& id, const std::string& fileName);
	const sf::Font&					getFont(const std::string& id) const;
	const sf::Font&					getDefaultFont() const;

private:
	std::unordered_map<std::string, sf::Texture>	m_textures;
	std::unordered_map<std::string, sf::Font>		m_fonts;
};

