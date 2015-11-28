/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2012 by the Widelands Development Team
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

#include "editor/tools/editor_place_bob_tool.h"

#include "editor/editorinteractive.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
*/
int32_t EditorPlaceBobTool::handle_click_impl(Widelands::Map& map,
                                                 const Widelands::World& world,
                                                 Widelands::NodeAndTriangle<> const center,
                                                 EditorInteractive& parent,
                                                 EditorActionArgs& args) {

	if (get_nr_enabled() && args.obob_type.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), args.sel_radius));
		do {
			Widelands::Bob * const mbob = mr.location().field->get_first_bob();
			args.obob_type.push_back((mbob ? &mbob->descr() : nullptr));
			args.nbob_type.push_back(world.get_bob_descr(get_random_enabled()));
		} while (mr.advance(map));
	}

	if (!args.nbob_type.empty()) {
		Widelands::EditorGameBase & egbase = parent.egbase();
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), args.sel_radius));
		std::list< const Widelands::BobDescr * >::iterator i = args.nbob_type.begin();
		do {
			const Widelands::BobDescr & descr = *(*i);
			if (mr.location().field->nodecaps() & descr.movecaps()) {
				if (Widelands::Bob * const bob = mr.location().field->get_first_bob())
					bob->remove(egbase); //  There is already a bob. Remove it.
				descr.create(egbase, nullptr, mr.location());
			}
			++i;
		} while (mr.advance(map));
		return mr.radius() + 2;
	} else
		return 0;
}

int32_t
EditorPlaceBobTool::handle_undo_impl(Widelands::Map& map,
                                        const Widelands::World&,
                                        Widelands::NodeAndTriangle<Widelands::Coords> center,
                                        EditorInteractive& parent,
                                        EditorActionArgs& args) {
	if (!args.nbob_type.empty()) {
		Widelands::EditorGameBase & egbase = parent.egbase();
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), args.sel_radius));
		std::list<const Widelands::BobDescr *>::iterator i = args.obob_type.begin();
		do {
			if (*i) {
				const Widelands::BobDescr & descr = *(*i);
				if (mr.location().field->nodecaps() & descr.movecaps()) {
					if (Widelands::Bob * const bob = mr.location().field->get_first_bob())
						bob->remove(egbase); //  There is already a bob. Remove it.
					descr.create(egbase, nullptr, mr.location());
				}
			} else if (Widelands::Bob * const bob = mr.location().field->get_first_bob()) {
				bob->remove(egbase);
			}
			++i;
		} while (mr.advance(map));
		return mr.radius() + 2;
	} else
		return 0;
}

EditorActionArgs EditorPlaceBobTool::format_args_impl(EditorInteractive & parent)
{
	return EditorTool::format_args_impl(parent);
}
