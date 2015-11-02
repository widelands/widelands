/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef WL_LOGIC_BUILDCOST_H
#define WL_LOGIC_BUILDCOST_H

#include <map>

#include "logic/widelands.h"

class FileRead;
class FileWrite;

namespace Widelands {

class TribeDescr;

struct Buildcost : std::map<WareIndex, uint8_t> {

	uint32_t total() const;

	void save(FileWrite & fw, const TribeDescr & tribe) const;
	void load(FileRead & fw, const TribeDescr & tribe);
};

} // namespace Widelands

#endif  // end of include guard: WL_LOGIC_BUILDCOST_H
