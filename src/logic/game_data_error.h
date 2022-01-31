/*
 * Copyright (C) 2009-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_GAME_DATA_ERROR_H
#define WL_LOGIC_GAME_DATA_ERROR_H

#include "base/wexception.h"

namespace Widelands {

/// Exception that is thrown when game data (world/tribe definitions, maps,
/// savegames or replays) are erroneous.
struct GameDataError : public WException {
	explicit GameDataError(char const* fmt, ...) PRINTF_FORMAT(2, 3);

	char const* what() const noexcept override {
		return what_.c_str();
	}

protected:
	GameDataError() {
	}
};

/** This exception's message compiles information for the user when an old savegame could not be
 * loaded due to packet version mismatch.
 *
 * The main message is localizeable, the technical information is not.
 */
struct UnhandledVersionError : public GameDataError {

	/** CTor
	 *
	 * @param packet_version         The version of the packet that Widelands is trying to load.
	 * @param current_packet_version The packet version that Widelands is currently using.
	 */
	explicit UnhandledVersionError(const char* packet_name,
	                               int32_t packet_version,
	                               int32_t current_packet_version);

	char const* what() const noexcept override {
		return what_.c_str();
	}

protected:
	UnhandledVersionError() {
	}
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_GAME_DATA_ERROR_H
