/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef TRIBEBASICINFO_H
#define TRIBEBASICINFO_H

#include <string>
#include <vector>

#include <stdint.h>

/// Basic information about the tribe that is determined only from the conf
/// file and needed before the actual game.
struct TribeBasicInfo {
	std::string name;

	/// Relative position of this tribe in a list of tribes.
	uint8_t uiposition;

	typedef std::pair<std::string, std::string> Initialization;
	typedef std::vector<Initialization> Initializations;
	Initializations initializations;
};

#endif
