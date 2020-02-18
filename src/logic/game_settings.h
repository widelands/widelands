/*
 * Copyright (C) 2008-2019 by the Widelands Development Team
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

#ifndef WL_LOGIC_GAME_SETTINGS_H
#define WL_LOGIC_GAME_SETTINGS_H

#include <memory>
#include <string>
#include <vector>

#include "io/filesystem/layered_filesystem.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player_end_result.h"
#include "logic/widelands.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

// PlayerSlot 0 will give us Widelands::PlayerNumber 1 etc., so we rename it to avoid confusion.
// TODO(GunChleoc): Rename all uint8_t to PlayerSlot or Widelands::PlayerNumber
using PlayerSlot = Widelands::PlayerNumber;

struct PlayerSettings {
	enum class State { kOpen, kHuman, kComputer, kClosed, kShared };

	/// Returns whether the given state allows sharing a slot at all
	static bool can_be_shared(PlayerSettings::State state) {
		return state != PlayerSettings::State::kClosed && state != PlayerSettings::State::kShared;
	}

	State state;
	uint8_t initialization_index;
	std::string name;
	std::string tribe;
	bool random_tribe;
	std::string ai; /**< Preferred AI provider for this player */
	bool random_ai;
	Widelands::TeamNumber team;
	bool closeable;     // only used in multiplayer scenario maps
	uint8_t shared_in;  // the number of the player that uses this player's starting position
};

struct UserSettings {
	// TODO(k.halfman): make this some const instead of calculating this every time
	static uint8_t none() {
		return std::numeric_limits<uint8_t>::max();
	}
	static uint8_t not_connected() {
		return none() - 1;
	}
	static uint8_t highest_playernum() {
		return not_connected() - 1;
	}

	UserSettings(Widelands::PlayerEndResult init_result, bool init_ready)
	   : result(init_result), ready(init_ready) {
	}
	UserSettings() : UserSettings(Widelands::PlayerEndResult::kUndefined, false) {
	}

	uint8_t position = 0;
	std::string name;
	Widelands::PlayerEndResult result;
	std::string win_condition_string;
	bool ready;  // until now only used as a check for whether user is currently receiving a file or
	             // not
};

/// The gamehost/gameclient are sending those to notify about status changes, which are then picked
/// up by the UI.
struct NoteGameSettings {
	CAN_BE_SENT_AS_NOTE(NoteId::GameSettings)

	enum class Action {
		kUser,    // A client has picked a different player slot / become an observer
		kPlayer,  // A player slot has changed its status (type, tribe etc.)
		kMap      // A new map/savegame was selected
	};

	Action action;
	PlayerSlot position;
	uint8_t usernum;

	explicit NoteGameSettings(Action init_action,
	                          PlayerSlot init_position = std::numeric_limits<uint8_t>::max(),
	                          uint8_t init_usernum = UserSettings::none())
	   : action(init_action), position(init_position), usernum(init_usernum) {
	}
};

/**
 * Holds all settings about a game that can be configured before the
 * game actually starts.
 *
 * Think of it as the Model in MVC.
 */
struct GameSettings {
	GameSettings()
	   : playernum(0),
	     usernum(0),
	     scenario(false),
	     multiplayer(false),
	     savegame(false),
	     peaceful(false) {
		std::unique_ptr<LuaInterface> lua(new LuaInterface);
		std::unique_ptr<LuaTable> win_conditions(
		   lua->run_script("scripting/win_conditions/init.lua"));
		for (const int key : win_conditions->keys<int>()) {
			std::string filename = win_conditions->get_string(key);
			if (g_fs->file_exists(filename)) {
				win_condition_scripts.push_back(filename);
			} else {
				throw wexception("Win condition file \"%s\" does not exist", filename.c_str());
			}
		}
	}

	/// Find a player number that the slot could share in. Does not guarantee that a viable slot was
	/// actually found.
	Widelands::PlayerNumber find_shared(PlayerSlot slot) const;
	/// Check if the player number returned by find_shared is usable
	bool is_shared_usable(PlayerSlot slot, Widelands::PlayerNumber shared) const;
	/// Savegame slots and certain scenario slots can't be closed
	bool uncloseable(PlayerSlot slot) const;

	/// Number of player position
	int16_t playernum;
	/// Number of users entry
	int8_t usernum;

	/// Name of the selected map
	std::string mapname;
	std::string mapfilename;

	/// Lua file defining the win condition to use.
	std::string win_condition_script;
	/// An ordered list of all win condition script files.
	std::vector<std::string> win_condition_scripts;

	/// Is map a scenario
	bool scenario;

	/// Is this a multiplayer game
	bool multiplayer;

	/// Is a savegame selected for loading?
	bool savegame;

	// Is all fighting forbidden?
	bool peaceful;

	/// List of tribes that players are allowed to choose
	std::vector<Widelands::TribeBasicInfo> tribes;

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
	virtual ~GameSettingsProvider() {
	}

	virtual const GameSettings& settings() = 0;

	virtual void set_scenario(bool set) = 0;
	virtual bool can_change_map() = 0;
	virtual bool can_change_player_state(uint8_t number) = 0;
	virtual bool can_change_player_tribe(uint8_t number) = 0;
	virtual bool can_change_player_init(uint8_t number) = 0;
	virtual bool can_change_player_team(uint8_t number) = 0;

	virtual bool can_launch() = 0;

	virtual void set_map(const std::string& mapname,
	                     const std::string& mapfilename,
	                     uint32_t maxplayers,
	                     bool savegame = false) = 0;
	virtual void set_player_state(uint8_t number, PlayerSettings::State) = 0;
	virtual void set_player_ai(uint8_t number, const std::string&, bool const random_ai = false) = 0;
	// Multiplayer no longer toggles per button
	virtual void next_player_state(uint8_t /* number */) {
	}
	virtual void
	set_player_tribe(uint8_t number, const std::string&, bool const random_tribe = false) = 0;
	virtual void set_player_init(uint8_t number, uint8_t index) = 0;
	virtual void set_player_name(uint8_t number, const std::string&) = 0;
	virtual void set_player(uint8_t number, const PlayerSettings&) = 0;
	virtual void set_player_number(uint8_t number) = 0;
	virtual void set_player_team(uint8_t number, Widelands::TeamNumber team) = 0;
	virtual void set_player_closeable(uint8_t number, bool closeable) = 0;
	virtual void set_player_shared(PlayerSlot number, Widelands::PlayerNumber shared) = 0;
	virtual void set_win_condition_script(const std::string& wc) = 0;
	virtual std::string get_win_condition_script() = 0;

	virtual void set_peaceful_mode(bool peace) = 0;
	virtual bool is_peaceful_mode() = 0;

	bool has_players_tribe() {
		return UserSettings::highest_playernum() >= settings().playernum;
	}
	// For retrieving tips texts
	struct NoTribe {};
	const std::string& get_players_tribe() {
		if (!has_players_tribe())
			throw NoTribe();
		return settings().players[settings().playernum].tribe;
	}
};

#endif  // end of include guard: WL_LOGIC_GAME_SETTINGS_H
