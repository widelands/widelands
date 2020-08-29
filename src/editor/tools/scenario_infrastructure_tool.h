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

#ifndef WL_EDITOR_TOOLS_SCENARIO_INFRASTRUCTURE_TOOL_H
#define WL_EDITOR_TOOLS_SCENARIO_INFRASTRUCTURE_TOOL_H

#include <vector>

#include "editor/tools/tool.h"
#include "logic/widelands.h"

struct ScenarioInfrastructureDeleteTool : public EditorTool {
	ScenarioInfrastructureDeleteTool() : EditorTool(*this, *this, false) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_infra_del.png");
	}
};

// Place buildings, flags and player immovables
struct ScenarioInfrastructureTool : public EditorTool {
	ScenarioInfrastructureTool(ScenarioInfrastructureDeleteTool& t)
	   : EditorTool(t, t, false), player_(1), index_(), force_(true), construct_(false) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_infra.png");
	}
	bool has_size_one() const override {
		return true;
	}

	const std::vector<std::pair<Widelands::MapObjectType, Widelands::DescriptionIndex>>&
	get_indices() const {
		return index_;
	}
	std::vector<std::pair<Widelands::MapObjectType, Widelands::DescriptionIndex>>& get_indices() {
		return index_;
	}
	Widelands::PlayerNumber get_player() const {
		return player_;
	}
	void set_player(Widelands::PlayerNumber p) {
		player_ = p;
	}
	bool get_construct() const {
		return construct_;
	}
	void set_construct(bool p) {
		construct_ = p;
	}
	bool get_force() const {
		return force_;
	}
	void set_force(bool f) {
		force_ = f;
	}

private:
	Widelands::PlayerNumber player_;
	std::vector<std::pair<Widelands::MapObjectType, Widelands::DescriptionIndex>> index_;
	bool force_;
	bool construct_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SCENARIO_INFRASTRUCTURE_TOOL_H
