/*
 * Copyright (C) 2002, 2006-2009, 2011, 2014-2015 by the Widelands Development Team
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

#ifndef WL_WUI_MAPTABLE_H
#define WL_WUI_MAPTABLE_H

#include <vector>

#include "ui_basic/table.h"
#include "wui/mapdata.h"


/**
 * A table listing all the available maps for saveloading.
 */
class MapTable : public UI::Table<uintptr_t> {
public:
	MapTable(UI::Panel * parent,
				 int32_t x, int32_t y, uint32_t w, uint32_t h,
				 const bool descending);

	/// Get the current selected map. Returns nullptr if there is nothing selected.
	const MapData* get_map() const;

	/// Fill the table with maps and directories.
	/// If localize_map_names = true, map names will be localized.
	/// Directory names are always localized.
	void fill(std::vector<MapData> entries, bool localize_map_names);

private:
	bool compare_players(uint32_t, uint32_t);
	bool compare_mapnames(uint32_t, uint32_t);
	bool compare_size(uint32_t, uint32_t);

	bool localize_map_names_;
	std::vector<MapData> maps_data_;
};

#endif  // end of include guard: WL_WUI_MAPTABLE_H
