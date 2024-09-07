# Belote with AI

## Table of Contents

  - [Overview](#overview)
  - [Features](#features)
    - [Game Engine & UI](#game-engine--ui)
    - [AI](#ai)
    - [Game Logic](#game-logic)
    - [TODO](#todo)
  - [License](#license)
  - [Demo](#demo)

## Overview

Welcome to **Belote**, a work-in-progress project that brings the popular French card game to life using **C++20** and **SFML** for a graphical interface. This implementation introduces an AI that leverages two key algorithms:

-   **Monte Carlo Tree Search (MCTS)** for playing cards, allowing the AI to simulate potential future plays and choose the optimal card.
-   **Heuristic Hill Climbing** for the bidding phase, enabling the AI to make intelligent decisions when selecting trump suits.

The combination of these algorithms aims to create a competitive AI that can play Belote strategically, making this a perfect project for AI enthusiasts and card game lovers alike.

## Features

### Game Engine & UI

-   **Minimalistic Game Engine** built with **SFML** for handling core game mechanics.
- **Minimalistic UI Framework** (used in earlier versions):
	-   Support for **2D components** with textures and text.
	-   Features include **centering**, **mouse interaction**, **hover states**, and **animation** for moving components.
	-   Allows specifying the **draw order** of UI components for layering.
-   **TGUI Integration**: UI reimplemented using **TGUI** for smoother interaction and better visuals.
-   **Human Playable**: The game is fully playable by human players with basic UI and card play validation.
-   **Debug UI** with all cards revealed

### AI

-   **Monte Carlo Tree Search (MCTS) AI**: Implements generic MCTS for trick playing, initially knowing all opponent cards, but now working with hidden cards randomized for simulation.
-   **Heuristic Hill Climbing AI for Bidding**: AI capable of smart decision-making during bidding based on Hill Climbing heuristic.

### Game Logic

-   **Belote Rules Implementation**: Comprehensive game logic that handles trick-taking, trump selection, scoring, and other Belote-specific rules.
-   **Card Play Validation**: Ensures players can only play valid cards during their turn according to the rules of Belote.

### TODO
- **AI Improvements**:
	-  Decision-making during **bidding** does not consider double/redouble at the moment
	-  **MCTS** tweakings and improvements for smarter trick playing
   	-  [WIP] randomize other player's card for the **MCTS step**
		-  Currently cards are randomized
		-  Use **weighted random** based on already played cards by other players, contract voting history, etc.  
	- Refactoring of the AI code 
- **Gameplay**
	- Declarations (announcements)
- Implement **Rules screen** for the main menu 

## License
This project is licensed under the MIT License. See the [LICENSE](./LICENSE.txt) file for details.

## Demo

![Alt text](belote.gif?raw=true "Belote")