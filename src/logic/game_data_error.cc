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

#include "logic/game_data_error.h"

#include <cstdarg>

#include "base/i18n.h"
#include "base/string.h"

namespace Widelands {

GameDataError::GameDataError(char const* const fmt, ...) {
	char buffer[512];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	what_ = buffer;
}

UnhandledVersionError::UnhandledVersionError(const char* packet_name,
                                             int32_t packet_version,
                                             int32_t current_packet_version) {
	what_ = format(
	   "\n\nUnhandledVersionError: %s\n\nPacket Name: %s\nSaved Version: %i\nCurrent "
	   "Version: %i",
	   _("This game was saved using an older version of Widelands and cannot be loaded anymore, "
	     "or it’s a new version that can’t be handled yet."),
	   packet_name, static_cast<int>(packet_version), static_cast<int>(current_packet_version));
}
}  // namespace Widelands
