/*
 * Copyright (C) 2008-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <string>
#include <vector>

#include "TribeBasicInfo.h"
#include "logic/widelands.h"

namespace Widelands {
enum class PlayerEndResult : uint8_t;
}

struct PlayerSettings {
	enum State {
		stateOpen,
		stateHuman,
		stateComputer,
		stateClosed,
		stateShared
	};

	State state;
	uint8_t     initialization_index;
	std::string name;
	std::string tribe;
	bool random_tribe;
	std::string ai; /**< Preferred AI provider for this player */
	bool random_ai;
	Widelands::TeamNumber team;
	bool closeable; // only used in multiplayer scenario maps
	uint8_t shared_in; // the number of the player that uses this player's starting position
};

struct UserSettings {
	static uint8_t none() {return std::numeric_limits<uint8_t>::max();}
	static uint8_t notConnected() {return none() - 1;}
	static uint8_t highestPlayernum() {return notConnected() - 1;}

	uint8_t     position;
	std::string name;
	Widelands::PlayerEndResult     result;
	std::string win_condition_string;
	bool        ready; // until now only used as a check for whether user is currently receiving a file or not
};

struct DedicatedMapInfos {
	std::string path;
	uint8_t     players;
	bool        scenario;
};

/**
 * Holds all settings about a game that can be configured before the
 * game actually starts.
 *
 * Think of it as the Model in MVC.
 */
struct GameSettings {
	GameSettings() :
		playernum(0),
		usernum(0),
		scenario(false),
		multiplayer(false),
		savegame(false)
	{}

	/// Number of player position
	int16_t playernum;
	/// Number of users entry
	int8_t usernum;

	/// Name of the selected map
	std::string mapname;
	std::string mapfilename;

	/// Lua file defining the win condition to use.
	std::string win_condition_script;

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

	/// Only used for dedicated servers so the clients can look through the maps available on the server
	/// like in their "own" map / saved games selection menu
	std::vector<DedicatedMapInfos> maps;
	std::vector<DedicatedMapInfos> saved_games;
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

	virtual const GameSettings & settings() = 0;

	virtual void setScenario(bool set) = 0;
	virtual bool canChangeMap() = 0;
	virtual bool canChangePlayerState(uint8_t number) = 0;
	virtual bool canChangePlayerTribe(uint8_t number) = 0;
	virtual bool canChangePlayerInit (uint8_t number) = 0;
	virtual bool canChangePlayerTeam (uint8_t number) = 0;

	virtual bool canLaunch() = 0;

	virtual void setMap
		(const std::string & mapname,
		 const std::string & mapfilename,
		 uint32_t maxplayers,
		 bool                savegame = false)
		= 0;
	virtual void setPlayerState    (uint8_t number, PlayerSettings::State) = 0;
	virtual void setPlayerAI       (uint8_t number, const std::string &, bool const random_ai = false) = 0;
	virtual void nextPlayerState   (uint8_t number) = 0;
	virtual void setPlayerTribe    (uint8_t number, const std::string &, bool const random_tribe = false) = 0;
	virtual void setPlayerInit     (uint8_t number, uint8_t index) = 0;
	virtual void setPlayerName     (uint8_t number, const std::string &) = 0;
	virtual void setPlayer         (uint8_t number, PlayerSettings) = 0;
	virtual void setPlayerNumber   (uint8_t number) = 0;
	virtual void setPlayerTeam     (uint8_t number, Widelands::TeamNumber team) = 0;
	virtual void setPlayerCloseable(uint8_t number, bool closeable) = 0;
	virtual void setPlayerShared   (uint8_t number, uint8_t shared) = 0;
	virtual void setWinConditionScript   (std::string wc) = 0;
	virtual void nextWinCondition      () = 0;
	virtual std::string getWinConditionScript() = 0;

	struct No_Tribe {};
	const std::string & getPlayersTribe() {
		if (UserSettings::highestPlayernum() < settings().playernum)
			throw No_Tribe();
		return settings().players[settings().playernum].tribe;
	}
};


#endif
