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

#ifndef BUILDCOST_H
#define BUILDCOST_H

#include <map>

#include "logic/widelands.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

struct Section;

namespace Widelands {

struct Tribe_Descr;

struct Buildcost : std::map<Ware_Index, uint8_t> {
	void parse(const Tribe_Descr & tribe, Section & buildcost_s);

	uint32_t total() const;

	void save(FileWrite & fw, const Tribe_Descr & tribe) const;
	void load(FileRead & fw, const Tribe_Descr & tribe);
};

} // namespace Widelands

#endif // BUILDCOST_H
