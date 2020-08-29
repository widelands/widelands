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

#ifndef WL_EDITOR_TOOLS_SCENARIO_ROAD_TOOL_H
#define WL_EDITOR_TOOLS_SCENARIO_ROAD_TOOL_H

#include <list>
#include <memory>

#include "editor/tools/tool.h"

struct ScenarioDeleteRoadTool : public EditorTool {
	explicit ScenarioDeleteRoadTool() : EditorTool(*this, *this, false) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_delete_road.png");
	}
};

struct ScenarioPlaceRoadTool : public EditorTool {
	explicit ScenarioPlaceRoadTool(ScenarioDeleteRoadTool& tool)
	   : EditorTool(tool, tool, false),
	     mode_(EditorActionArgs::RoadMode::kNormal),
	     create_primary_worker_(true),
	     create_secondary_worker_(true),
	     force_(false),
	     place_flags_(false) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map*) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_place_road.png");
	}
	bool has_size_one() const override {
		return true;
	}

	EditorActionArgs::RoadMode get_mode() const {
		return mode_;
	}
	bool get_force() const {
		return force_;
	}
	bool get_place_flags() const {
		return place_flags_;
	}
	bool get_create_primary_worker() const {
		return create_primary_worker_;
	}
	bool get_create_secondary_worker() const {
		return create_secondary_worker_;
	}

	void set_mode(EditorActionArgs::RoadMode m) {
		mode_ = m;
	}
	void set_force(bool f) {
		force_ = f;
	}
	void set_place_flags(bool f) {
		place_flags_ = f;
	}
	void set_create_primary_worker(bool c) {
		create_primary_worker_ = c;
	}
	void set_create_secondary_worker(bool c) {
		create_secondary_worker_ = c;
	}

private:
	EditorActionArgs::RoadMode mode_;
	bool create_primary_worker_;
	bool create_secondary_worker_;
	bool force_;
	bool place_flags_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SCENARIO_WORKER_TOOL_H
