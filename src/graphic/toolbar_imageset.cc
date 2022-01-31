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

#include "graphic/toolbar_imageset.h"

#include "graphic/image_cache.h"

ToolbarImageset::ToolbarImageset()
   : bottom_left_corner(g_image_cache->get("images/wui/toolbar/left_corner.png")),
     bottom_left(g_image_cache->get("images/wui/toolbar/left.png")),
     bottom_center(g_image_cache->get("images/wui/toolbar/center.png")),
     bottom_right(g_image_cache->get("images/wui/toolbar/right.png")),
     bottom_right_corner(g_image_cache->get("images/wui/toolbar/right_corner.png")),

     top_left_corner(g_image_cache->get("images/wui/toolbar/left_corner_inv.png")),
     top_left(g_image_cache->get("images/wui/toolbar/left_inv.png")),
     top_center(g_image_cache->get("images/wui/toolbar/center_inv.png")),
     top_right(g_image_cache->get("images/wui/toolbar/right_inv.png")),
     top_right_corner(g_image_cache->get("images/wui/toolbar/right_corner_inv.png")) {
}

ToolbarImageset::ToolbarImageset(const LuaTable& table)
   : bottom_left_corner(g_image_cache->get(table.get_string("bottom_left_corner"))),
     bottom_left(g_image_cache->get(table.get_string("bottom_left"))),
     bottom_center(g_image_cache->get(table.get_string("bottom_center"))),
     bottom_right(g_image_cache->get(table.get_string("bottom_right"))),
     bottom_right_corner(g_image_cache->get(table.get_string("bottom_right_corner"))),

     top_left_corner(g_image_cache->get(table.get_string("top_left_corner"))),
     top_left(g_image_cache->get(table.get_string("top_left"))),
     top_center(g_image_cache->get(table.get_string("top_center"))),
     top_right(g_image_cache->get(table.get_string("top_right"))),
     top_right_corner(g_image_cache->get(table.get_string("top_right_corner"))) {
}
