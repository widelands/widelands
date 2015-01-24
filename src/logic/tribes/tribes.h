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

#ifndef WL_LOGIC_TRIBES_TRIBES_H
#define WL_LOGIC_TRIBES_TRIBES_H

#include <memory>

#include "base/macros.h"
#include "logic/description_maintainer.h"
#include "scripting/lua_table.h"

constexpr const Widelands::WareIndex kInvalidWare = -1;

namespace Widelands {

class WareDescr;
class WorkerDescr;

// NOCOM(#sirver): Make this compatible with TribeDescr.
class Tribes {
public:
	Tribes();
	~Tribes();

	/// Adds this ware type to the tribe description.
	void add_ware_type(const LuaTable& table);

	/// Adds this worker type to the tribe description.
	void add_worker_type(const LuaTable& table);

	// NOCOM(#GunChleoc): Do we want all wares for all tribes here?
	WareIndex get_nrwares() const;
	WareIndex get_nrworkers() const;

	WareIndex safe_ware_index(const std::string & warename) const;
	WareIndex safe_worker_index(const std::string & workername) const;

	WareIndex ware_index(const std::string & warename) const;
	WareIndex worker_index(const std::string & workername) const;

	WareDescr const * get_ware_descr(WareIndex ware_index) const;
	WorkerDescr const * get_worker_descr(WareIndex worker_index) const;

	void set_ware_type_has_demand_check(WareIndex ware_index, const std::string& tribename);
	void set_worker_type_has_demand_check(WareIndex worker_index, const std::string& tribename);


private:
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;
	std::unique_ptr<DescriptionMaintainer<WorkerDescr>> workers_;

	DISALLOW_COPY_AND_ASSIGN(Tribes);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRIBES_TRIBES_H
