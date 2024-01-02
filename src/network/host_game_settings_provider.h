/*
 * Copyright (C) 2021-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_NETWORK_HOST_GAME_SETTINGS_PROVIDER_H
#define WL_NETWORK_HOST_GAME_SETTINGS_PROVIDER_H

#include "logic/game_settings.h"
#include "network/gamehost.h"

class HostGameSettingsProvider : public GameSettingsProvider {
public:
	explicit HostGameSettingsProvider(GameHost* const init_host) : host_(init_host) {
	}
	~HostGameSettingsProvider() override = default;

	void set_scenario(bool is_scenario) override;

	const GameSettings& settings() override;

	bool can_change_map() override;
	bool can_change_player_state(uint8_t number) override;
	bool can_change_player_tribe(uint8_t) override;
	bool can_change_player_init(uint8_t) override;
	bool can_change_player_team(uint8_t) override;
	bool can_launch() override;

	void set_map(const std::string& mapname,
	             const std::string& mapfilename,
	             const std::string& map_theme,
	             const std::string& map_bg,
	             uint32_t maxplayers,
	             bool savegame) override;

	void set_player_state(uint8_t number, PlayerSettings::State state) override;
	void set_player_ai(uint8_t number, const std::string& name, bool random_ai) override;
	void set_player_tribe(uint8_t number, const std::string& tribe, bool random_tribe) override;
	void set_player_init(uint8_t number, uint8_t index) override;
	void set_player_team(uint8_t number, Widelands::TeamNumber team) override;
	void set_player_color(uint8_t number, const RGBColor&) override;
	void set_player_closeable(uint8_t, bool) override;
	void set_player_shared(PlayerSlot, Widelands::PlayerNumber) override;
	void set_player_name(uint8_t number, const std::string& name) override;
	void set_player(uint8_t number, const PlayerSettings& ps) override;
	void set_player_number(uint8_t number) override;

	std::string get_win_condition_script() override;
	void set_win_condition_script(const std::string& wc) override;
	int32_t get_win_condition_duration() override;
	void set_win_condition_duration(int32_t duration) override;

	void set_flag(GameSettings::Flags flag, bool state) override;
	bool get_flag(GameSettings::Flags flag) override;

private:
	GameHost* host_;
	std::vector<std::string> wincondition_scripts_;
};

#endif  // end of include guard: WL_NETWORK_HOST_GAME_SETTINGS_PROVIDER_H
