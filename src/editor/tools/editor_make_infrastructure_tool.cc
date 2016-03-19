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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include "editor/tools/editor_make_infrastructure_tool.h"

#include "editor/editorinteractive.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/player.h"
#include "wui/fieldaction.h"

/**
 * Callback function to calculate correct overlays
 */
int32_t
editor_make_infrastructure_tool_callback
	(const Widelands::TCoords<Widelands::FCoords>& c,
	 Widelands::EditorGameBase& egbase,
	 int32_t const player)
{
	return egbase.player(player).get_buildcaps(c);
}


/**
 * This is the most complex of all tool functions: check where was clicked,
 * offer the correct user dialog and act accordingly.
 *
 * Obviously, this function ignores the sel radius
*/
int32_t EditorMakeInfrastructureTool::handle_click_impl(const Widelands::World&,
                                                        Widelands::NodeAndTriangle<> const,
                                                        EditorInteractive& parent,
                                                        EditorActionArgs* /* args */,
														Widelands::Map*) {
	show_field_action
	(&parent, parent.egbase().get_player(player_), &registry_);

	//  Not really needed, since Player Immovables are taken care of while
	//  placing on map.
	return 5;
}
