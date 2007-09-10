/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

/*
 * Callback function to calculate correct overlays
 */
int Editor_Make_Infrastructure_Tool_Callback
(const TCoords<FCoords> c, void * data, int player)
{
	return
		static_cast<const Editor_Game_Base *>(data)->player(player).get_buildcaps(c);
}


/*
===========
Editor_Make_Infrastructure_Tool::handle_click_impl()

This is the most complex of all tool functions: check where was clicked,
offer the correct user dialog and act accordingly.

Obviously, this function ignores the sel radius
===========
*/
int Editor_Make_Infrastructure_Tool::handle_click_impl
(Map &, const Node_and_Triangle<>, Editor_Interactive & parent)
{
	// Special case for buildings
	/*BaseImmovable * const imm = map.get_immovable(get_sel_pos());


	if (imm && imm->get_type() == Map_Object::BUILDING) {
		Building *building = (Building *)imm;
      building->show_options(this);
	}
*/

	show_field_action
		(&parent, parent.editor().get_player(m_player), &m_registry);
   return 5; // Not really needed, since Player Immovables are taken care of while placing on map
}
