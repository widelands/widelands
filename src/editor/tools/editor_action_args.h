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

#ifndef WL_EDITOR_TOOLS_EDITOR_ACTION_ARGS_H
#define WL_EDITOR_TOOLS_EDITOR_ACTION_ARGS_H

#include <list>
#include <string>

#include "logic/field.h"
#include "logic/widelands_geometry.h"

namespace Widelands {
class BobDescr;
}  // namespace Widelands

struct EditorInteractive;
struct EditorToolAction;

/// Class to save important and changeable properties of classes needed for actions
// Implementations in editor_history.cc
struct EditorActionArgs {
	EditorActionArgs(EditorInteractive & base);

	// TODO(sirver): This class does its own reference counting. This design is
	// brittle and on a quick overview I have a feeling that it might not be
	// correct.
	EditorActionArgs(const EditorActionArgs&) = default;
	EditorActionArgs& operator = (const EditorActionArgs&) = default;

	~EditorActionArgs();

	uint32_t sel_radius;

	int32_t change_by;                                              // resources, hight change tools
	std::list<Widelands::Field::Height> origHights;                 // change hight tool
	uint8_t cur_res, set_to;                                        // resources change tools
	std::list<uint8_t> orgRes, orgResT;                             // resources set tool
	std::list<const Widelands::BobDescr *> obob_type, nbob_type;  // bob change tools
	std::list<std::string> oimmov_types;                            // immovable change tools
	std::list<int32_t> nimmov_types;                                // immovable change tools
	Widelands::HeightInterval m_interval;                  // noise hight tool
	std::list<Widelands::DescriptionIndex> terrainType, origTerrainType; // set terrain tool

	std::list<EditorToolAction *> draw_actions;                   // draw tool

	uint32_t refcount;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_EDITOR_ACTION_ARGS_H
