/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "editor/tools/scenario_vision_tool.h"

#include "editor/editorinteractive.h"
#include "logic/mapregion.h"
#include "logic/widelands_geometry.h"

void ScenarioVisionTool::set_player(EditorInteractive& eia, uint8_t p) {
	assert(p);
	player_ = p;
	eia.set_illustrating_vision_for(p);
}

int32_t ScenarioVisionTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                              EditorInteractive& eia,
                                              EditorActionArgs* args,
                                              Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	Widelands::Player& player = *eia.egbase().get_player(args->new_owner);
	do {
		if (args->vision == Widelands::SeeUnseeNode::kUnsee) {
			// Unexplored fields need to be revealed and re-hidden
			player.hide_or_reveal_field(0, mr.location(), Widelands::SeeUnseeNode::kReveal);
		}
		player.hide_or_reveal_field(0, mr.location(), args->vision);
	} while (mr.advance(*map));
	return mr.radius();
}

EditorActionArgs ScenarioVisionTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_owner = player_;
	a.vision = mode_;
	return a;
}
