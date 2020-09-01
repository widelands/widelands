/*
 * Copyright (C) 2019-2020 by the Widelands Development Team
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

#include "graphic/toolbar_imageset.h"

#include "graphic/image_cache.h"

ToolbarImageset::ToolbarImageset()
   : left_corner(g_image_cache->get("images/wui/toolbar/left_corner.png")),
     left(g_image_cache->get("images/wui/toolbar/left.png")),
     center(g_image_cache->get("images/wui/toolbar/center.png")),
     right(g_image_cache->get("images/wui/toolbar/right.png")),
     right_corner(g_image_cache->get("images/wui/toolbar/right_corner.png")) {
}

ToolbarImageset::ToolbarImageset(const LuaTable& table)
   : left_corner(g_image_cache->get(table.get_string("left_corner"))),
     left(g_image_cache->get(table.get_string("left"))),
     center(g_image_cache->get(table.get_string("center"))),
     right(g_image_cache->get(table.get_string("right"))),
     right_corner(g_image_cache->get(table.get_string("right_corner"))) {
}
