/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_LOGIC_TRIBE_TRIBE_H
#define WL_LOGIC_TRIBE_TRIBE_H

#include <memory>

#include "base/macros.h"
#include "logic/description_maintainer.h"
#include "scripting/lua_table.h"

namespace Widelands {

class WareDescr;

// NOCOM(#sirver): Make this compatible with TribeDescr.
class Tribe {
public:
	Tribe();
	~Tribe();

	/// Adds this ware type to the tribe description.
	void add_ware_type(const LuaTable& table);

private:
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;

	DISALLOW_COPY_AND_ASSIGN(Tribe);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRIBE_TRIBE_H
