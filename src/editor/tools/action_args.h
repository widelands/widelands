/*
 * Copyright (C) 2012-2022 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_ACTION_ARGS_H
#define WL_EDITOR_TOOLS_ACTION_ARGS_H

#include "logic/field.h"
#include "logic/map.h"
#include "logic/widelands_geometry.h"

namespace Widelands {
class BobDescr;
}  // namespace Widelands

class EditorInteractive;
struct EditorToolAction;
enum class WindowID;

/// Class to save important and changeable properties of classes needed for actions
// Implementations in editor_history.cc
struct EditorActionArgs {
	explicit EditorActionArgs(EditorInteractive& base);

	// TODO(sirver): This class does its own reference counting. This design is
	// brittle and on a quick overview I have a feeling that it might not be
	// correct.
	EditorActionArgs(const EditorActionArgs&) = default;
	EditorActionArgs& operator=(const EditorActionArgs&) = default;

	~EditorActionArgs();

	uint32_t sel_radius;

	int32_t change_by;                                     // resources, change height tools
	std::list<Widelands::Field::Height> original_heights;  // change height tool
	Widelands::DescriptionIndex current_resource;          // resources change tools
	Widelands::ResourceAmount set_to;                      // resources change tools
	Widelands::Extent new_map_size;                        // resize tool

	struct ResourceState {
		Widelands::Coords location;
		Widelands::DescriptionIndex idx;
		Widelands::ResourceAmount amount;
	};

	std::list<ResourceState> original_resource;                        // resources set tool
	std::list<const Widelands::BobDescr*> old_bob_type, new_bob_type;  // bob change tools
	std::list<std::string> old_immovable_types;                        // immovable change tools
	std::list<Widelands::DescriptionIndex> new_immovable_types;        // immovable change tools
	Widelands::HeightInterval interval;                                // noise height tool
	std::list<Widelands::DescriptionIndex> terrain_type, original_terrain_type;  // set terrain tool
	Widelands::ResizeHistory resized;                                            // resize tool

	std::list<EditorToolAction*> draw_actions;  // draw tool

	uint32_t refcount;

	WindowID window_id;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_ACTION_ARGS_H
