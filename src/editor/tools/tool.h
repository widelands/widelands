/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_TOOL_H
#define WL_EDITOR_TOOLS_TOOL_H

#define MAX_TOOL_AREA 9

#include "base/macros.h"
#include "editor/tools/action_args.h"
#include "graphic/image.h"
#include "logic/editor_game_base.h"
#include "logic/widelands_geometry.h"

/**
 * An editor tool is a tool that can be selected in the editor. Examples are:
 * modify height, place immovable, place critter, place building. A Tool only
 * makes one function (like delete_building, place building, modify building
 * are 3 tools).
 */
class EditorTool {
public:
	EditorTool(EditorTool& second, EditorTool& third, bool uda = true)
	   : second_(second), third_(third), undoable_(uda) {
	}
	virtual ~EditorTool() {
	}

	enum ToolIndex { First, Second, Third };
	int32_t handle_click(ToolIndex i,
	                     const Widelands::NodeAndTriangle<>& center,
	                     EditorInteractive& parent,
	                     EditorActionArgs* args,
	                     Widelands::Map* map) {
		return (i == First  ? *this :
		        i == Second ? second_ :
                            third_)
		   .handle_click_impl(center, parent, args, map);
	}

	int32_t handle_undo(ToolIndex i,
	                    const Widelands::NodeAndTriangle<>& center,
	                    EditorInteractive& parent,
	                    EditorActionArgs* args,
	                    Widelands::Map* map) {
		return (i == First  ? *this :
		        i == Second ? second_ :
                            third_)
		   .handle_undo_impl(center, parent, args, map);
	}

	const Image* get_sel(const ToolIndex i) {
		return (i == First ? *this : i == Second ? second_ : third_).get_sel_impl();
	}

	EditorActionArgs format_args(const ToolIndex i, EditorInteractive& parent) {
		return (i == First ? *this : i == Second ? second_ : third_).format_args_impl(parent);
	}

	bool is_undoable() {
		return undoable_;
	}
	virtual bool has_size_one() const {
		return false;
	}
	virtual EditorActionArgs format_args_impl(EditorInteractive& parent) {
		return EditorActionArgs(parent);
	}
	virtual int32_t handle_click_impl(const Widelands::NodeAndTriangle<>&,
	                                  EditorInteractive&,
	                                  EditorActionArgs*,
	                                  Widelands::Map*) = 0;
	virtual int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>&,
	                                 EditorInteractive&,
	                                 EditorActionArgs*,
	                                 Widelands::Map*) {
		return 0;
	}  // non unduable tools don't need to implement this.
	virtual const Image* get_sel_impl() const = 0;

	// Gives the tool the chance to modify the nodecaps to change what will be
	// displayed as build help.
	virtual Widelands::NodeCaps nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
	                                                   const Widelands::EditorGameBase&) {
		return fcoords.field->nodecaps();
	}

	virtual bool operates_on_triangles() const {
		return false;
	}

protected:
	EditorTool &second_, &third_;
	bool undoable_;

private:
	DISALLOW_COPY_AND_ASSIGN(EditorTool);
};

#endif  // end of include guard: WL_EDITOR_TOOLS_TOOL_H
