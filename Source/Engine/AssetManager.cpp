#include "AssetManager.h"

void AssetManager::initializeDefaultAssets()
{
	// Don't do this in the constructor because it happens before a sf::RenderWindow is created and triggers an OpenGL error
	loadTexture("MissingTexture", "assets/missing_texture.png");
	loadFont("DefaultFont", "assets/fonts/arial.ttf");
}

void AssetManager::loadTexture(const std::string& id, const std::string& fileName)
{
	sf::Texture texture;

	if (texture.loadFromFile(fileName))
	{
		m_textures[id] = texture;
	}
}

const sf::Texture& AssetManager::getTexture(const std::string& id) const
{
	return hasTexture(id) ? m_textures.at(id) : m_textures.at("MissingTexture"); // this will throw an exception if the MissingTexture is not found
}

bool AssetManager::hasTexture(const std::string& id) const
{
	return m_textures.count(id) > 0;
}

void AssetManager::loadFont(const std::string& id, const std::string& fileName)
{
	sf::Font font;

	if (font.loadFromFile(fileName))
	{
		m_fonts[id] = font;
	}
}

const sf::Font& AssetManager::getFont(const std::string& id) const
{
	return m_fonts.count(id) > 0 ? m_fonts.at(id) : m_fonts.at("DefaultFont"); // this will throw an exception if the DefaultFont is not found
}

const sf::Font& AssetManager::getDefaultFont() const
{
	return m_fonts.at("DefaultFont"); // this will throw an exception if the DefaultFont is not found
}
