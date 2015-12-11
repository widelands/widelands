/*
 * Copyright (C) 2002-2003, 2006-2009 by the Widelands Development Team
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

#ifndef WL_LOGIC_WARE_DESCR_H
#define WL_LOGIC_WARE_DESCR_H

#include <cstring>
#include <string>
#include <unordered_map>

#include <stdint.h>

#include "base/macros.h"
#include "logic/instances.h"
#include "scripting/lua_table.h"

class Image;
class LuaTable;

#define WARE_MENU_PIC_WIDTH   24  //< Default width for ware's menu icons
#define WARE_MENU_PIC_HEIGHT  24  //< Default height for ware's menu icons
#define WARE_MENU_PIC_PAD_X    3  //< Default padding between menu icons
#define WARE_MENU_PIC_PAD_Y    4  //< Default padding between menu icons

namespace Widelands {

class TribeDescr;

/**
 * Wares can be stored in warehouses. They can be transferred across an
 * Economy. They can be traded.
*/
class WareDescr : public MapObjectDescr {
public:
	WareDescr(const std::string& init_descname, const LuaTable& t);
	~WareDescr() override {}

	/// Returns the preciousness of the ware, or kInvalidWare if the tribe doesn't use the ware.
	/// It is used by the computer player.
	int preciousness(const std::string& tribename) const;

	/// How much of the ware type an economy should store in warehouses.
	/// The special value kInvalidWare means that the target quantity of this ware type will never be checked
  ///  and should not be configurable.
	DescriptionIndex default_target_quantity(const std::string& tribename) const;

	std::string helptext_script() const {return helptext_script_;}

	bool has_demand_check(const std::string& tribename) const;

	/// Called when a demand check for this ware type is encountered during
	/// parsing. If there was no default target quantity set in the ware type's
	/// configuration for the 'tribename', sets the default value to 1.
	void set_has_demand_check(const std::string& tribename);

	// Add a building to the list of consumers
	void add_consumer(const DescriptionIndex& building_index);
	// Add a building to the list of producers
	void add_producer(const DescriptionIndex& building_index);

	// The buildings that consume this ware
	const std::set<DescriptionIndex>& consumers() const;
	// The buildings that produce this ware
	const std::set<DescriptionIndex>& producers() const;

private:
	// tribename, quantity. No default.
	std::unordered_map<std::string, int> default_target_quantities_;
	// tribename, preciousness. No default.
	std::unordered_map<std::string, int> preciousnesses_;

	std::set<DescriptionIndex> consumers_; // Buildings that consume this ware
	std::set<DescriptionIndex> producers_; // Buildings that produce this ware

	std::string  helptext_script_;  // The path and filename to the ware's helptext script
	DISALLOW_COPY_AND_ASSIGN(WareDescr);
};

}

#endif  // end of include guard: WL_LOGIC_WARE_DESCR_H
