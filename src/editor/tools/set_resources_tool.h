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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_EDITOR_TOOLS_SET_RESOURCES_TOOL_H
#define WL_EDITOR_TOOLS_SET_RESOURCES_TOOL_H

#include "editor/tools/tool.h"

Widelands::NodeCaps resource_tools_nodecaps(const Widelands::FCoords& fcoords,
                                            const Widelands::EditorGameBase& egbase,
                                            Widelands::DescriptionIndex resource);

///  Decreases the resources of a node by a value.
struct EditorSetResourcesTool : public EditorTool {
	EditorSetResourcesTool(EditorInteractive& parent)
	   : EditorTool(parent, *this, *this), cur_res_(0), set_to_(0) {
	}

	/**
	 * Sets the resources of the current to a fixed value
	 */
	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorActionArgs* args,
	                         Widelands::Map* map) override;

	EditorActionArgs format_args_impl() override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_set_resources.png");
	}

	Widelands::NodeCaps nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
	                                           const Widelands::EditorGameBase& egbase) override {
		return resource_tools_nodecaps(fcoords, egbase, cur_res_);
	}

	Widelands::ResourceAmount get_set_to() const {
		return set_to_;
	}
	void set_set_to(Widelands::ResourceAmount const n) {
		set_to_ = n;
	}
	Widelands::DescriptionIndex get_cur_res() const {
		return cur_res_;
	}
	void set_cur_res(Widelands::DescriptionIndex const res) {
		cur_res_ = res;
	}

	WindowID get_window_id() override {
		return WindowID::ChangeResources;
	}

	bool save_configuration_impl(ToolConf& conf) override {
		conf.resource = cur_res_;
		conf.set_to = set_to_;
		return true;
	}
	void load_configuration(const ToolConf& conf) override {
		cur_res_ = conf.resource;
		set_to_ = conf.set_to;
	}

private:
	Widelands::DescriptionIndex cur_res_;
	Widelands::ResourceAmount set_to_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SET_RESOURCES_TOOL_H
