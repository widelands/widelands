/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#include "editor_set_origin_tool.h"

#include "editor/editorinteractive.h"
#include "logic/map.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/overlay_manager.h"

int32_t Editor_Set_Origin_Tool::handle_click_impl
	(Widelands::Map               &       map,
	 Widelands::Node_and_Triangle<> const center,
	 Editor_Interactive           &       eia)
{
	map.set_origin(center.node);
	map.overlay_manager().reset();
	eia.register_overlays();
	eia.set_rel_viewpoint
		(Point
		 	(-(center.node.x * 2 + (center.node.y & 1)) * (TRIANGLE_WIDTH / 2),
		 	 - center.node.y *                             TRIANGLE_HEIGHT));
	return 0;
}
