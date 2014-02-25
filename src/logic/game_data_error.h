/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#ifndef GAME_DATA_ERROR_H
#define GAME_DATA_ERROR_H

#include "wexception.h"

namespace Widelands {

/// Exceptiont that is thrown when game data (world/tribe definitions, maps,
/// savegames or replays) are erroneous.
struct game_data_error : public _wexception {
	explicit game_data_error(char const * fmt, ...) PRINTF_FORMAT(2, 3);

	virtual char const * what() const throw () override {return m_what.c_str();}
protected:
	game_data_error() {};
};

}

#endif
