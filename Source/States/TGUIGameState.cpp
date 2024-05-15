#include "TGUIGameState.h"

#include "TGUISplashState.h"

#include "Application.h"
#include "GameState.h"
#include "TGUI/Backend/Window/SFML/BackendGuiSFML.hpp"
#include "TGUI/Widgets/Picture.hpp"
#include "TGUI/Widgets/Button.hpp"
#include "TGUI/Widgets/Label.hpp"

#include "TGUI/Widgets/Group.hpp"
#include "TGUI/Widgets/HorizontalWrap.hpp"
#include "TGUI/Widgets/HorizontalLayout.hpp"
#include "TGUI/Widgets/Panel.hpp"

namespace
{
	const unsigned int					SCREEN_WIDTH = 1600;
	const unsigned int					SCREEN_HEIGHT = 900;
	const sf::Vector2f					SCREEN_CENTER = { SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f };
	const float							SCREEN_PADDING = 35.f;

	const sf::Vector2f					CARD_SCALE{ 0.4f, 0.4f };
	const float							CARD_WIDTH = 222.f * CARD_SCALE.x;
	const float							CARD_HEIGHT = 323.f * CARD_SCALE.y;
	const float							VERTICAL_CARDS_OVERLAP = 0.66f;
	const float							HORIZONTAL_CARDS_OVERLAP = 0.f;

	const float							DECK_PADDING_OFFSET_PERCENT = 0.01f;
	tgui::Layout2d						DECK_POSITION{ "1%", "1%" };
	const float							DECK_CARDS_OFFSET = 9;

	const int							CARD_DEALING_TIME_MS = 750;
	const int							TRICK_END_WAIT_TIME_MS = 2000;
	const float							AI_BIDDING_WAIT_TIME = 2.f;
	const float							CONTRACT_VOTE_LABEL_TIMEOUT = 2.f;
	const float							WAIT_TIME_AFTER_PLAYING = 2.f;

	const char							BG_COLOR_GREEN[] = "#33b249";
	const char							BG_COLOR_RED[] = "#dd7973";
}

TGUIGameState::TGUIGameState(StateMachine& stateMachine, bool human /*= false*/)
	: BaseState(stateMachine)
	, m_gui(Application::getInstance()->getWindow())
{
	if (human)
	{
		m_belote.getPlayers()[1]->setHuman(true);
	}

	m_gui.add(tgui::Picture::create("assets/background.jpg"), "background");

	m_pauseLabel = tgui::Label::create("PAUSE");
	m_pauseLabel->setTextSize(78);
	m_pauseLabel->getRenderer()->setTextColor(sf::Color::Red);
	m_pauseLabel->setPosition("50% - width/2", "50%");
	m_pauseLabel->setVisible(false);
	m_gui.add(m_pauseLabel, "pause");

	createDeck();
	createPlayerNames();
	createInfoPanel();
	createContractVoteLabel();

	updateInfoPanel();
	updateActivePlayerLabel();

	static_cast<Subject<NotifyCardDealing>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyContractVote>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyCardAboutToBePlayed>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyEndOfTrick>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyEndOfRound>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyNewRound>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyContractVoteRequired>&>(*Application::getInstance()).registerObserver(*this);
	static_cast<Subject<NotifyPlayCardRequired>&>(*Application::getInstance()).registerObserver(*this);
	//testWidgetPositions();
}

void TGUIGameState::createDeck()
{
	size_t cardIndex = 0;
	for (const Card* card : m_belote.getDeck())
	{
		auto cardWidget = tgui::Picture::create(tgui::Texture(card->getTexturePath()));
		cardWidget->setScale(CARD_SCALE);
		cardWidget->setPosition(getCardPositionInDeck(cardIndex));
		m_gui.add(cardWidget, card->toString());
		++cardIndex;
	}
}

void TGUIGameState::createPlayerNames()
{
	for (const auto& player_ptr : m_belote.getPlayers())
	{
		auto label = tgui::Label::create(player_ptr->getNameForUI());
		label->setTextSize(28);
		label->getRenderer()->setBackgroundColor(BG_COLOR_GREEN);
		label->getRenderer()->setBorders(1);
		const float offsetX = player_ptr->getPlayerIndex() == 2 ? label->getSize().x : 0;
		const float offsetY = player_ptr->getPlayerIndex() == 1 ? label->getSize().y : 0;

		label->setPosition(getPlayerNamePosition(*player_ptr, -offsetX, -offsetY));
		m_gui.add(label, player_ptr->getNameForUI());
		m_playerNameLabels.push_back(label);
	}
}

void TGUIGameState::createInfoPanel()
{
	auto infoPanel = tgui::Panel::create();

	auto pos1 = getPlayerNamePosition(*m_belote.getPlayers()[0]);
	auto pos2 = getCardPositionInPlayer(*m_belote.getPlayers()[0], 0);
	infoPanel->setPosition({ pos1.x, pos2.y });
	infoPanel->setSize({ CARD_WIDTH * 3 + 2 * DECK_CARDS_OFFSET, CARD_HEIGHT });
	infoPanel->getRenderer()->setBackgroundColor(BG_COLOR_GREEN);
	infoPanel->getRenderer()->setBorders(1);
	m_gui.add(infoPanel, "info_panel");

	m_scoreLabel = tgui::Label::create(std::format("Total score: {} - {}", 0, 0));
	m_scoreLabel->setTextSize(24);
	infoPanel->add(m_scoreLabel, "total_score");

	m_contractLabel = tgui::Label::create(std::format("Contract: {}", ""));
	m_contractLabel->setTextSize(16);
	m_contractLabel->setPosition({ 0, m_scoreLabel->getSize().y });
	infoPanel->add(m_contractLabel, "contract");

	m_contractCallerLabel = tgui::Label::create(std::format("Called by: {}", ""));
	m_contractCallerLabel->setTextSize(16);
	m_contractCallerLabel->setPosition({ 0, m_scoreLabel->getSize().y + m_contractLabel->getSize().y });
	infoPanel->add(m_contractCallerLabel, "contract_caller");

	auto size = m_scoreLabel->getSize() + m_contractLabel->getSize() + m_contractCallerLabel->getSize();
	infoPanel->setSize({ infoPanel->getSize().x, size.y });
}

void TGUIGameState::createContractVoteLabel()
{
	m_contractVoteLabel = tgui::Label::create("N/A");
	m_contractVoteLabel->setTextSize(28);
	m_contractVoteLabel->getRenderer()->setBackgroundColor(BG_COLOR_GREEN);
	m_contractVoteLabel->getRenderer()->setBorders(1);
	m_contractVoteLabel->setPosition(getContractVoteLabelPosition(m_belote.getActivePlayer()));
	m_contractVoteLabel->setVisible(false);
	m_gui.add(m_contractVoteLabel);
}

void TGUIGameState::updateInfoPanel()
{
	m_scoreLabel->setText(std::format("Total score: {} - {}", m_belote.getTeamScore(0), m_belote.getTeamScore(1)));

	const Contract* contract = m_belote.anyRoundPlayed() ? &m_belote.getCurrentRound().getBiddingManager().getContract() : nullptr;
	const std::string contract_str = contract && contract->getType() != Contract::Type::Invalid ? contract->toString() : "";
	m_contractLabel->setText("Contract: " + contract_str);
	
	const std::string caller_str = contract && contract->getPlayer() ? contract->getPlayer()->getNameForUI() : "";
	m_contractCallerLabel->setText("Called by: " + caller_str);
}

void TGUIGameState::updateActivePlayerLabel(const Player* player /*= nullptr*/)
{
	for (auto& label : m_playerNameLabels)
	{
		const bool isActive = player && label->getWidgetName() == player->getNameForUI();
		label->getRenderer()->setBackgroundColor(isActive ? BG_COLOR_RED : BG_COLOR_GREEN);
	}
}

void TGUIGameState::updateContractVoteLabel(const Contract* contractVote /*= nullptr*/)
{
	m_contractVoteLabel->finishAllAnimations();
	m_contractVoteLabel->setVisible(contractVote != nullptr);

	if (contractVote)
	{
		m_contractVoteLabel->setText(contractVote->toString());
		m_contractVoteLabel->setPosition(getContractVoteLabelPosition(*contractVote->getPlayer()));
		m_contractVoteLabel->hideWithEffect(tgui::ShowEffectType::Fade, CONTRACT_VOTE_LABEL_TIMEOUT * 1000 * 1.2);
	}
}

tgui::Layout2d TGUIGameState::getCardPositionInDeck(size_t cardIndex)
{
	const size_t maxCardsPerRow = (SCREEN_WIDTH - 2 * SCREEN_WIDTH * DECK_PADDING_OFFSET_PERCENT) / (CARD_WIDTH + DECK_CARDS_OFFSET);
	const size_t rowIndex = cardIndex / maxCardsPerRow;
	const size_t colIndex = cardIndex % maxCardsPerRow;

	tgui::Layout x = DECK_POSITION.x + colIndex * (CARD_WIDTH + DECK_CARDS_OFFSET);
	tgui::Layout y = DECK_POSITION.y + rowIndex * (CARD_HEIGHT + DECK_CARDS_OFFSET);
	return { x, y };
}

tgui::Layout2d TGUIGameState::getCardPositionInPlayer(const Player& player, size_t cardIndex)
{
	switch (player.getPlayerIndex())
	{
	case 0: // Left
		{
			tgui::Layout2d pos = getCardPositionInDeck(16);
			pos.y = pos.y + CARD_HEIGHT + SCREEN_PADDING + cardIndex * (CARD_HEIGHT * (1 - VERTICAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	case 1: // Bottom
		{
			tgui::Layout2d pos = getCardPositionInDeck(20);
			pos.y = SCREEN_HEIGHT - CARD_HEIGHT - SCREEN_PADDING;
			pos.x = pos.x + cardIndex * (CARD_WIDTH * (1 - HORIZONTAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	case 2: // Right
		{
			tgui::Layout2d pos = getCardPositionInDeck(31);
			pos.y = pos.y + CARD_HEIGHT + SCREEN_PADDING + cardIndex * (CARD_HEIGHT * (1 - VERTICAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	case 3: // Up
		{
			tgui::Layout2d pos = getCardPositionInDeck(20);
			pos.y = pos.y + CARD_HEIGHT + SCREEN_PADDING;
			pos.x = pos.x + cardIndex * (CARD_WIDTH * (1 - HORIZONTAL_CARDS_OVERLAP) + DECK_CARDS_OFFSET);
			return pos;
		}
		break;
	}

	return {};
}

tgui::Layout2d TGUIGameState::getCardPositionOnTable(const Player& player)
{
	const Player& player1 = *m_belote.getPlayers()[1];
	const Player& player3 = *m_belote.getPlayers()[3];

	switch (player.getPlayerIndex())
	{
	case 0: // Left
	{
		tgui::Layout2d pos1 = getCardPositionInPlayer(player1, 2);
		tgui::Layout2d pos3 = getCardPositionInPlayer(player3, 2);
		pos1.y = pos3.y + (pos1.y - pos3.y) / 2;
		return pos1;
	}
	break;
	case 1: // Bottom
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player1, 3);
		pos.y = pos.y - CARD_HEIGHT - DECK_CARDS_OFFSET;
		pos.x = pos.x + (CARD_WIDTH + DECK_CARDS_OFFSET) / 2;
		return pos;
	}
	break;
	case 2: // Right
	{
		tgui::Layout2d pos1 = getCardPositionInPlayer(player1, 5);
		tgui::Layout2d pos3 = getCardPositionInPlayer(player3, 5);
		pos1.y = pos3.y + (pos1.y - pos3.y) / 2;
		return pos1;
	}
	break;
	case 3: // Up
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player3, 3);
		pos.y = pos.y + CARD_HEIGHT + DECK_CARDS_OFFSET;
		pos.x = pos.x + (CARD_WIDTH + DECK_CARDS_OFFSET) / 2;
		return pos;
	}
	break;
	}

	return {};
}

tgui::Layout2d TGUIGameState::getPlayerNamePosition(const Player& player, float offsetX /*= 0*/, float offsetY /*= 0*/)
{
	switch (player.getPlayerIndex())
	{
	case 0: // Left
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player, 4);
		pos.x = pos.x + CARD_WIDTH + DECK_CARDS_OFFSET + offsetX;
		return pos;
	}
	break;
	case 1: // Bottom
	{
		//tgui::Layout2d pos = getCardPositionOnTable(player);
		//pos.y = SCREEN_HEIGHT - SCREEN_PADDING;
		//return pos;
		tgui::Layout2d pos = getCardPositionInPlayer(player, 0);
		pos.y = pos.y - DECK_CARDS_OFFSET + offsetY;
		return pos;
	}
	break;
	case 2: // Right
	{
		tgui::Layout2d pos = getCardPositionInPlayer(player, 4);
		pos.x = pos.x - DECK_CARDS_OFFSET + offsetX;
		return pos;
	}
	break;
	case 3: // Up
	{
		//tgui::Layout2d pos = getCardPositionOnTable(player);
		//pos.y = pos.y - CARD_HEIGHT - DECK_CARDS_OFFSET - SCREEN_PADDING;
		//return pos;
		tgui::Layout2d pos = getCardPositionInPlayer(player, 0);
		pos.y = pos.y + DECK_CARDS_OFFSET + CARD_HEIGHT;
		return pos;
	}
	break;
	}

	return {};
}

tgui::Layout2d TGUIGameState::getContractVoteLabelPosition(const Player& player)
{
	auto label = m_gui.get(player.getNameForUI());
	if (!label) return {};

	auto pos = label->getPosition();
	if (player.getPlayerIndex() == 2)
	{
		const float offsetX = m_contractVoteLabel->getSize().x;
		return { pos.x - DECK_CARDS_OFFSET - offsetX, pos.y }; // Right player: label is on the left of their name
	}

	return { pos.x + DECK_CARDS_OFFSET + label->getSize().x, pos.y };
}

void TGUIGameState::handleInput()
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
		else if (sf::Event::KeyReleased == event.type && event.key.code == sf::Keyboard::Key::F5)
		{
			togglePause();
		}
		else if (sf::Event::KeyReleased == event.type && event.key.code == sf::Keyboard::Key::F12)
		{
			m_gui.removeAllWidgets();
			testWidgetPositions();
		}
	}
}

void TGUIGameState::update(float dtSeconds)
{
	if (m_delayGameSeconds > 0.f)
	{
		m_delayGameSeconds -= dtSeconds;
	}

	// Note: The following check is for widgets attached to root. We need it for card movement
	const bool isPlayingAnyAnimation = std::any_of(m_gui.getWidgets().begin(), m_gui.getWidgets().end(), [](tgui::Widget::Ptr widget)
	{
		return widget->isAnimationPlaying();
	});
	const bool shouldPause = m_delayGameSeconds > 0.f || m_shouldPauseGame || isPlayingAnyAnimation;
	if (!shouldPause)
	{
		m_belote.updateState();
		updateActivePlayerLabel(&m_belote.getActivePlayer());
	}
}

void TGUIGameState::draw()
{
	sf::RenderWindow& window = (sf::RenderWindow&)*m_gui.getTarget();

	window.clear({ 240, 240, 240 });
	m_gui.draw();
	window.display();
}

void TGUIGameState::notify(const NotifyCardDealing& data)
{
	auto cardWidget = m_gui.get(data.m_card.toString());
	auto pos = getCardPositionInPlayer(data.m_player, data.m_player.getCards().size());
	cardWidget->moveToFront();
	cardWidget->moveWithAnimation(pos, CARD_DEALING_TIME_MS);
}

void TGUIGameState::notify(const NotifyContractVoteRequired& data)
{
	if (!data.m_player.isHuman())
	{
		delayGame(AI_BIDDING_WAIT_TIME);
	}
	else
	{
		showContractOptions();
	}
}

void TGUIGameState::notify(const NotifyContractVote& data)
{
	updateInfoPanel();
	updateContractVoteLabel(&data.m_contract);
}

void TGUIGameState::notify(const NotifyCardAboutToBePlayed& data)
{
	auto cardWidget = m_gui.get(data.m_card.toString());
	auto pos = getCardPositionOnTable(data.m_player);
	cardWidget->moveWithAnimation(pos, CARD_DEALING_TIME_MS);
	delayGame(WAIT_TIME_AFTER_PLAYING);
}

void TGUIGameState::notify(const NotifyEndOfTrick& data)
{
	for (const Card* card : data.m_trick.getCards())
	{
		auto cardWidget = m_gui.get(card->toString());

		tgui::ShowEffectType effect = tgui::ShowEffectType::Fade;
		switch (data.m_trick.getWinningCardTurn()->m_player->getPlayerIndex())
		{
		case 0: effect = tgui::ShowEffectType::SlideToLeft; break;
		case 1: effect = tgui::ShowEffectType::SlideToBottom; break;
		case 2: effect = tgui::ShowEffectType::SlideToRight; break;
		case 3: effect = tgui::ShowEffectType::SlideToTop; break;
		}

		cardWidget->hideWithEffect(effect, TRICK_END_WAIT_TIME_MS);
	}
}

void TGUIGameState::notify(const NotifyEndOfRound& data)
{
	updateInfoPanel();
}

void TGUIGameState::notify(const NotifyNewRound& data)
{
	size_t cardIndex = 0;
	for (const Card* card : m_belote.getDeck())
	{
		auto cardWidget = m_gui.get(card->toString());
		cardWidget->finishAllAnimations();
		cardWidget->setPosition(getCardPositionInDeck(cardIndex));
		cardWidget->setVisible(true);
		++cardIndex;
	}
	updateInfoPanel();
}

void TGUIGameState::notify(const NotifyPlayCardRequired& data)
{
	if (!m_belote.getActivePlayer().isHuman())
	{
		return;
	}

	for (const Card* card : m_belote.getActivePlayer().getCards())
	{
		tgui::Picture::Ptr widget = m_gui.get<tgui::Picture>(card->toString());
		const float opacity = m_belote.getCurrentRound().getCurrentTrick().canPlayCard(*card) ? 1.f : 0.5f;
		widget->getRenderer()->setOpacity(opacity);

		widget->onMousePress([this, card]()
		{
			if (m_belote.getCurrentRound().getCurrentTrick().canPlayCard(*card))
			{
				for (const Card* card : m_belote.getActivePlayer().getCards())
				{
					tgui::Picture::Ptr widget = m_gui.get<tgui::Picture>(card->toString());
					widget->onMousePress.disconnectAll();
					widget->getRenderer()->setOpacity(1.f);
				}
				m_belote.getActivePlayer().playCard(*card);
			}
		});
	}
}

void TGUIGameState::delayGame(float seconds)
{
	m_delayGameSeconds = seconds;
}

void TGUIGameState::togglePause()
{
	m_shouldPauseGame = !m_shouldPauseGame;

	m_pauseLabel->setVisible(m_shouldPauseGame);
	m_pauseLabel->moveToFront();
}

void TGUIGameState::showContractOptions()
{
	std::vector<Contract> votes;

	for (int i = (int)Contract::Type::Pass; i <= (int)Contract::Type::AllTrumps; ++i)
	{
		votes.emplace_back(Contract::Type(i), &m_belote.getActivePlayer());		
	}

	const Contract& current = m_belote.getCurrentRound().getBiddingManager().getContract();

	votes.emplace_back(current.getType(), &m_belote.getActivePlayer(), Contract::Level::Double);
	votes.emplace_back(current.getType(), &m_belote.getActivePlayer(), Contract::Level::Redouble);

	for (size_t i = 0u; i < votes.size(); ++i)
	{
		auto button = tgui::Button::create(std::format("{}", votes[i].toString()));
		button->setSize(200, 25);
		button->setEnabled(m_belote.getCurrentRound().getBiddingManager().canBid(votes[i]));
		const std::string y = std::format("parent.height / 4 + {} * 5 + height * {} + 25%", i, i);
		button->setPosition({ "(parent.width - width) / 2", y.c_str() });
		button->onPress([this, contract = votes[i]]()
		{ 
			m_belote.getActivePlayer().setContractVote(contract);
			for (auto& button : m_contractVoteButtons)
			{
				m_gui.remove(button);
			}
			m_contractVoteButtons.clear();
		});
		m_gui.add(button, votes[i].toString());
		m_contractVoteButtons.push_back(button);
	}
}

void TGUIGameState::testWidgetPositions()
{
	m_gui.add(tgui::Picture::create("assets/background.jpg"), "background");

	createDeck();
	createPlayerNames();

	size_t cardIndex = 0;
	for (const Card* card : m_belote.getDeck())
	{
		auto cardWidget = tgui::Picture::create(tgui::Texture(card->getTexturePath()));
		cardWidget->setScale(CARD_SCALE);
		const size_t playerIndex = cardIndex / 8;
		cardWidget->setPosition(getCardPositionInPlayer(*m_belote.getPlayers()[playerIndex], cardIndex % 8));
		m_gui.add(cardWidget, card->toString());
		++cardIndex;
	}
	for (auto& player_ptr : m_belote.getPlayers())
	{
		const Card* card = m_belote.getDeck()[player_ptr->getPlayerIndex()];
		auto cardWidget = tgui::Picture::create(tgui::Texture(card->getTexturePath()));
		cardWidget->setScale(CARD_SCALE);
		cardWidget->setPosition(getCardPositionOnTable(*player_ptr));
		m_gui.add(cardWidget, card->toString());

		auto activeLabel = tgui::Label::create("ACTIVE");
		activeLabel->setTextSize(28);
		activeLabel->getRenderer()->setBackgroundColor(BG_COLOR_RED);
		activeLabel->getRenderer()->setBorders(1);
		activeLabel->setPosition(getContractVoteLabelPosition(*player_ptr));
		m_gui.add(activeLabel);
	}


	createInfoPanel();

}
