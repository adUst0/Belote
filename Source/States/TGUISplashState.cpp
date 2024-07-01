#include "TGUISplashState.h"

#include "Application.h"
#include "GameState.h"
#include "TGUI/Backend/Window/SFML/BackendGuiSFML.hpp"
#include "TGUI/Widgets/Picture.hpp"
#include "TGUI/Widgets/Button.hpp"
#include "TGUI/Widgets/Label.hpp"
#include "TGUIGameState.h"

namespace
{
	static constexpr size_t buttonOffset = 10u;
}

TGUISplashState::TGUISplashState(StateMachine& stateMachine)
	: BaseState(stateMachine)
	, m_gui(Application::getInstance()->getWindow())
{
	m_gui.add(tgui::Picture::create("assets/background.jpg"), "background");

	auto label = tgui::Label::create();
	label->setPosition({ "(parent.width - width) / 2", 60 });
	label->setText("Belote (Bulgarian rules)");
	label->setTextSize(72);
	m_gui.add(label);

	const size_t numButtons = 5;
	std::vector<tgui::Button::Ptr> buttons;
	buttons.reserve(numButtons);

	auto button = buttons.emplace_back(tgui::Button::create("New Game"));
	button->onPress([this]() { m_stateMachine.pushState(std::make_unique<TGUIGameState>(m_stateMachine, true, false)); });
	button->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 175));
	button->getRenderer()->setRoundedBorderRadius(50.f);

	/*button = buttons.emplace_back(tgui::Button::create("New Game (Debug TGUI)"));
	button->onPress([this]() { m_stateMachine.pushState(std::make_unique<TGUIGameState>(m_stateMachine)); });
	button->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 175));
	button->getRenderer()->setRoundedBorderRadius(50.f);*/

	button = buttons.emplace_back(tgui::Button::create("New Game Human (Debug TGUI)"));
	button->onPress([this]() { m_stateMachine.pushState(std::make_unique<TGUIGameState>(m_stateMachine, true, true)); });
	button->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 175));
	button->getRenderer()->setRoundedBorderRadius(50.f);

	/*button = buttons.emplace_back(tgui::Button::create("New Game (Debug Old)"));
	button->onPress([this]() { m_stateMachine.pushState(std::make_unique<GameState>(m_stateMachine)); });
	button->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 175));
	button->getRenderer()->setRoundedBorderRadius(50.f);*/

	button = buttons.emplace_back(tgui::Button::create("Rules"));
	button->onPress([this]() { std::cout << "Rules button clicked. TODO - not implemented" << std::endl; });
	button->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 175));
	button->getRenderer()->setRoundedBorderRadius(50.f);

	button = buttons.emplace_back(tgui::Button::create("Quit"));
	button->onPress([]() { Application::getInstance()->getWindow().close(); });
	button->getRenderer()->setBackgroundColor(sf::Color(255, 255, 255, 175));
	button->getRenderer()->setRoundedBorderRadius(50.f);

	//tgui::Texture texture("assets/button.png");
	//tgui::Texture textureHover("assets/button_hover.png");
	for (int i = 0; i < buttons.size(); ++i)
	{
		buttons[i]->setSize(300, 60);
		const std::string y = std::format("parent.height / 4 + {} * 10 + height * {}", i, i);
		buttons[i]->setPosition({ "(parent.width - width) / 2", y.c_str() });
		//buttons[i]->getRenderer()->setTexture(texture);
		//buttons[i]->getRenderer()->setTextureHover(textureHover);
		//buttons[i]->getRenderer()->setBorderColorHover({ 255, 255, 255, 0 });
		//buttons[i]->getRenderer()->setBorderColor({ 255, 255, 255, 0 });
		m_gui.add(buttons[i]);
	}
}

void TGUISplashState::handleInput()
{
	sf::RenderWindow& window = (sf::RenderWindow&)*m_gui.getTarget();

	sf::Event event;

	while (window.pollEvent(event))
	{
		m_gui.handleEvent(event);

		if (sf::Event::Closed == event.type)
		{
			window.close();
		}
	}
}

void TGUISplashState::draw()
{
	sf::RenderWindow& window = (sf::RenderWindow&)*m_gui.getTarget();

	window.clear({ 240, 240, 240 });
	m_gui.draw();
	window.display();
}
