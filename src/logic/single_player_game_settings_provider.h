/*
 * Copyright (C) 2015-2019 by the Widelands Development Team
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

#ifndef WL_LOGIC_SINGLE_PLAYER_GAME_SETTINGS_PROVIDER_H
#define WL_LOGIC_SINGLE_PLAYER_GAME_SETTINGS_PROVIDER_H

#include "logic/game_settings.h"

/**
 * The settings provider for normal singleplayer games:
 * The user can change everything, except that they are themselves human.
 */
struct SinglePlayerGameSettingsProvider : public GameSettingsProvider {
	SinglePlayerGameSettingsProvider();

	void set_scenario(bool const set) override;

	const GameSettings& settings() override;

	bool can_change_map() override;
	bool can_change_player_state(uint8_t number) override;
	bool can_change_player_tribe(uint8_t) override;
	bool can_change_player_init(uint8_t) override;
	bool can_change_player_team(uint8_t) override;
	bool can_launch() override;

	virtual std::string get_map();
	virtual void set_map(const std::string& mapname,
	                     const std::string& mapfilename,
	                     uint32_t const maxplayers,
	                     bool const savegame) override;

	void set_player_state(uint8_t const number, PlayerSettings::State state) override;
	void set_player_ai(uint8_t const number, const std::string& ai, bool const random_ai) override;
	void next_player_state(uint8_t const number) override;
	void
	set_player_tribe(uint8_t const number, const std::string& tribe, bool random_tribe) override;
	void set_player_init(uint8_t const number, uint8_t const index) override;
	void set_player_team(uint8_t number, Widelands::TeamNumber team) override;
	void set_player_closeable(uint8_t, bool) override;
	void set_player_shared(PlayerSlot, Widelands::PlayerNumber) override;
	void set_player_name(uint8_t const number, const std::string& name) override;
	void set_player(uint8_t const number, const PlayerSettings& ps) override;
	void set_player_number(uint8_t const number) override;

	std::string get_win_condition_script() override;
	void set_win_condition_script(const std::string& wc) override;

	void set_peaceful_mode(bool peace) override;
	bool is_peaceful_mode() override;

private:
	GameSettings s;
};

#endif  // end of include guard: WL_LOGIC_SINGLE_PLAYER_GAME_SETTINGS_PROVIDER_H
