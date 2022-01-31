/*
 * Copyright (C) 2019-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TOOLBAR_IMAGESET_H
#define WL_GRAPHIC_TOOLBAR_IMAGESET_H

#include "graphic/image.h"
#include "scripting/lua_table.h"

/// An imageset for a horizontal toolbar
struct ToolbarImageset {
	ToolbarImageset();
	explicit ToolbarImageset(const LuaTable& table);

	/// Will be painted beyond the left corner of the toolbar
	const Image* bottom_left_corner;
	/// Will be repeated between the left corner and the center
	const Image* bottom_left;
	/// Will be painted at the center
	const Image* bottom_center;
	/// Will be repeated between the right corner and the center
	const Image* bottom_right;
	/// Will be painted beyond the right corner of the toolbar
	const Image* bottom_right_corner;

	const Image* top_left_corner;
	const Image* top_left;
	const Image* top_center;
	const Image* top_right;
	const Image* top_right_corner;
};

#endif  // end of include guard: WL_GRAPHIC_TOOLBAR_IMAGESET_H
