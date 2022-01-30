/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_GAME_IO_GAME_PRELOAD_PACKET_H
#define WL_GAME_IO_GAME_PRELOAD_PACKET_H

#include <vector>

#include "base/times.h"
#include "game_io/game_data_packet.h"
#include "logic/addons.h"
#include "logic/game_controller.h"

namespace Widelands {

/**
 * This contains all the preload data needed to identify
 * a game for a user (for example in a listbox)
 */

struct GamePreloadPacket : public GameDataPacket {
	void read(FileSystem&, Game&, MapObjectLoader* = nullptr) override;
	void write(FileSystem&, Game&, MapObjectSaver* = nullptr) override;

	char const* get_mapname() const {
		return mapname_.c_str();
	}
	std::string get_background() const {
		return background_;
	}
	std::string get_background_theme() const {
		return background_theme_;
	}
	std::string get_win_condition() const {
		return win_condition_;
	}
	std::string get_localized_win_condition() const;
	const Time& get_gametime() const {
		return gametime_;
	}
	uint8_t get_player_nr() const {
		return player_nr_;
	}
	std::string get_version() const {
		return version_;
	}

	uint8_t get_number_of_players() const {
		return number_of_players_;
	}
	std::string get_minimap_path() const {
		return minimap_path_;
	}

	time_t get_savetimestamp() const {
		return savetimestamp_;
	}
	GameController::GameType get_gametype() const {
		return gametype_;
	}

	/* NOTE: Info about enabled *world* add-ons is saved in the Map by MapElemental
	 * packet, and additionally info about enabled *tribes and world* add-ons is
	 * saved in the GamePreload packet.
	 * On game loading, the game loader will dis- or enable *tribe and world* add-ons
	 * as requested by the GamePreload packet. That packet is also used to display
	 * compatibility warnings in the game loading screens.
	 * The add-on info stored in the Map(Elemental packet) is ignored during loading.
	 * The map selection screens in main menu and editor will need to take care
	 * to use the info stored there to dis- and enable *world* add-ons as required.
	 * As a side effect, the choice of *world* add-ons is left to the map maker,
	 * and players can not influence the world of existing maps.
	 * Tribes add-ons however are selected when starting a new game.
	 */
	const AddOns::AddOnRequirements& required_addons() const {
		return required_addons_;
	}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	const std::string& get_active_training_wheel() const {
		return active_training_wheel_;
	}

	bool get_training_wheels_wanted() const {
		return training_wheels_wanted_ || !active_training_wheel_.empty();
	}
#endif

private:
	std::string minimap_path_;
	std::string mapname_;
	std::string background_;
	std::string background_theme_;
	std::string win_condition_;
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	std::string active_training_wheel_;
	// Initializing everything to make cppcheck happy.
	bool training_wheels_wanted_ = false;
#endif
	Time gametime_ = Time(0);
	uint8_t player_nr_ = 0U;  // The local player idx
	uint8_t number_of_players_ = 0U;
	std::string version_;
	time_t savetimestamp_ = 0;
	GameController::GameType gametype_ = GameController::GameType::kUndefined;
	// Required add-ons with the recommended version
	AddOns::AddOnRequirements required_addons_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_GAME_IO_GAME_PRELOAD_PACKET_H
