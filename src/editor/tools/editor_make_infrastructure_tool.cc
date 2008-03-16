/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "editor_make_infrastructure_tool.h"

#include "editor_game_base.h"
#include "editorinteractive.h"
#include "field.h"
#include "fieldaction.h"
#include "map.h"
#include "player.h"

/**
 * Callback function to calculate correct overlays
 */
int32_t Editor_Make_Infrastructure_Tool_Callback
(Widelands::TCoords<Widelands::FCoords> const c, void * data, int32_t player)
{
	return
		static_cast<Widelands::Editor_Game_Base const *>(data)->player(player)
		.get_buildcaps(c);
}


/**
 * This is the most complex of all tool functions: check where was clicked,
 * offer the correct user dialog and act accordingly.
 *
 * Obviously, this function ignores the sel radius
*/
int32_t Editor_Make_Infrastructure_Tool::handle_click_impl
(Widelands::Map                     &,
 Widelands::Node_and_Triangle<> const,
 Editor_Interactive                 & parent)
{
	show_field_action
		(&parent, parent.egbase().get_player(m_player), &m_registry);

	//  Not really needed, since Player Immovables are taken care of while
	//  placing on map.
	return 5;
}
