/*
 * Copyright (C) 2012 by the Widelands Development Team
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

#ifndef EDITOR_ACTION_ARGS_H
#define EDITOR_ACTION_ARGS_H

#include <list>
#include <string>

#include "interval.h"
#include "logic/field.h"

namespace Widelands {
class BobDescr;
}  // namespace Widelands

struct Editor_Interactive;
struct Editor_Tool_Action;

/// Class to save important and changeable properties of classes needed for actions
// Implementations in editor_history.cc
struct Editor_Action_Args {
	uint32_t sel_radius;

	int32_t change_by;                                              // resources, hight change tools
	std::list<Widelands::Field::Height> origHights;                 // change hight tool
	uint8_t cur_res, set_to;                                        // resources change tools
	std::list<uint8_t> orgRes, orgResT;                             // resources set tool
	std::list<const Widelands::BobDescr *> obob_type, nbob_type;  // bob change tools
	std::list<std::string> oimmov_types;                            // immovable change tools
	std::list<int32_t> nimmov_types;                                // immovable change tools
	interval<Widelands::Field::Height> m_interval;                  // noise hight tool
	std::list<Widelands::Terrain_Index> terrainType, origTerrainType; // set terrain tool

	std::list<Editor_Tool_Action *> draw_actions;                   // draw tool

	Editor_Action_Args(Editor_Interactive & base);
	~Editor_Action_Args();
	uint32_t refcount;
};

#endif
