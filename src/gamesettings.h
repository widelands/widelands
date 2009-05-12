/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include "tribe.h"

#include <string>
#include <vector>

struct PlayerSettings {
	enum State {
		stateOpen,
		stateHuman,
		stateComputer,
		stateClosed
	};

	State state;
	uint8_t     initialization_index;
	std::string name;
	std::string tribe;
	std::string ai; /**< Preferred AI provider for this player */
};

struct UserSettings {
	int32_t     position; // -1 if in lobby
	std::string name;
};

/**
 * Holds all settings about a game that can be configured before the
 * game actually starts.
 *
 * Think of it as the Model in MVC.
 */
struct GameSettings {
	GameSettings() : savegame(false) {}

	/// Number of player position
	int32_t playernum;
	/// Number of users entry
	uint32_t usernum;

	/// Name of the selected map
	std::string mapname;
	std::string mapfilename;

	/// Is map a scenario
	bool scenario;

	/// Is this a multiplayer game
	bool multiplayer;

	/// Is a savegame selected for loading?
	bool savegame;

	/// List of tribes that players are allowed to choose
	std::vector<TribeBasicInfo> tribes;

	/// Player configuration, with 0-based indices for players
	std::vector<PlayerSettings> players;

	/// Users connected to the game (0-based indices) - only used in multiplayer
	std::vector<UserSettings> users;
};


/**
 * UI classes are given a GameSettingsProvider instead of direct
 * access to \ref GameSettings. This allows pluggable behaviour in menus,
 * depending on whether the menu was called for a singleplayer game or
 * multiplayer game.
 *
 * Think of it as a mix of Model and Controller in MVC.
 */
struct GameSettingsProvider {
	virtual ~GameSettingsProvider() {}

	virtual GameSettings const & settings() = 0;

	virtual void setScenario(bool set) = 0;
	virtual bool canChangeMap() = 0;
	virtual bool canChangePlayerState(uint8_t number) = 0;
	virtual bool canChangePlayerTribe(uint8_t number) = 0;
	virtual bool canChangePlayerInit (uint8_t number) = 0;

	virtual bool canLaunch() = 0;

	virtual void setMap
		(std::string const & mapname,
		 std::string const & mapfilename,
		 uint32_t maxplayers,
		 bool                savegame = false)
		= 0;
	virtual void setPlayerState (uint8_t number, PlayerSettings::State) = 0;
	virtual void setPlayerAI    (uint8_t number, std::string const &) = 0;
	virtual void nextPlayerState(uint8_t number) = 0;
	virtual void setPlayerTribe (uint8_t number, std::string const &) = 0;
	virtual void setPlayerInit  (uint8_t number, uint8_t index) = 0;
	virtual void setPlayerName  (uint8_t number, std::string const &) = 0;
	virtual void setPlayer      (uint8_t number, PlayerSettings) = 0;
	virtual void setPlayerNumber(int32_t number) = 0;
	const std::string getPlayersTribe() {
		if (settings().playernum < 0)
			return std::string();
		return settings().players[settings().playernum].tribe;
	}
};


#endif // GAMESETTINGS_H
