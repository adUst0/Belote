#pragma once
#include "StateMachine.h"
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "Belote.h"

class TGUIGameState : public BaseState
{
public:
	TGUIGameState(StateMachine& stateMachine);

	void createPlayerNames();

	void							createDeck();
	tgui::Layout2d					getCardPositionInDeck(size_t cardIndex);
	tgui::Layout2d					getCardPositionInPlayer(const Player& player, size_t cardIndex);
	tgui::Layout2d					getCardPositionOnTable(const Player& player);
	tgui::Layout2d					getPlayerNamePosition(const Player& player, float offsetX = 0, float offsetY = 0);

	virtual void					handleInput() override;
	virtual void					draw() override;

private:
	void							testWidgetPositions();

	tgui::Gui						m_gui;
	
	Belote							m_belote;

	float							m_delayGameSeconds = 0.f;
	bool							m_shouldPauseGame = false;
};


