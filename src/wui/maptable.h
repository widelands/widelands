/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_basic/table.h"
#include "wui/mapdata.h"

/**
 * A table listing all the available maps for saveloading.
 * This contains a UI model only; the callig classes have to define the data model
 * and bind the compare functions.
 */
class MapTable : public UI::Table<uintptr_t> {
public:
	MapTable(UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style);

	/// Fill the table with maps and directories.
	void fill(const std::vector<MapData>& entries, MapData::DisplayType type);
};

#endif  // end of include guard: WL_WUI_MAPTABLE_H
