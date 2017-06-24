/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_GAME_IO_GAME_PRELOAD_PACKET_H
#define WL_GAME_IO_GAME_PRELOAD_PACKET_H

#include <cstring>
#include <ctime>
#include <string>

#include "game_io/game_data_packet.h"

#include "logic/game_controller.h"

namespace Widelands {

/**
 * This contains all the preload data needed to identify
 * a game for a user (for example in a listbox)
 */

struct GamePreloadPacket : public GameDataPacket {
	GamePreloadPacket();

	void read(FileSystem&, Game&, MapObjectLoader* = nullptr) override;
	void write(FileSystem&, Game&, MapObjectSaver* = nullptr) override;

	char const* get_mapname() const {
		return mapname_.c_str();
	}
	std::string get_background() const {
		return background_;
	}
	std::string get_win_condition() const {
		return win_condition_;
	}
	std::string get_localized_win_condition() const;
	uint32_t get_gametime() const {
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

private:
	std::string minimap_path_;
	std::string mapname_;
	std::string background_;
	std::string win_condition_;
	uint32_t gametime_;
	uint8_t player_nr_;  // The local player idx
	uint8_t number_of_players_;
	std::string version_;
	time_t savetimestamp_;
	GameController::GameType gametype_;
};
}

#endif  // end of include guard: WL_GAME_IO_GAME_PRELOAD_PACKET_H
