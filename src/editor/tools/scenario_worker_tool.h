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

#ifndef WL_EDITOR_TOOLS_SCENARIO_WORKER_TOOL_H
#define WL_EDITOR_TOOLS_SCENARIO_WORKER_TOOL_H

#include <list>

#include "editor/tools/tool.h"
#include "logic/map_objects/tribes/worker_descr.h"

/* NOCOM: Replace this with a tool to open an options window with
 * · an experience spinbox (all workers),
 * · sliders for training levels and health points (soldiers),
 * · a shipname editbox (ships), and
 * · a Delete button (all).
 * Or perhaps keep this tool and implement such functionality
 * into the infrastructure settings window instead?
 * How should the case of multiple workers per tile be handled?
 */
// Place and delete workers and ships.
struct ScenarioDeleteWorkerTool : public EditorTool {
	explicit ScenarioDeleteWorkerTool() : EditorTool(*this, *this) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
	                         Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_gr->images().get("images/wui/editor/fsel_editor_delete_worker.png");
	}
};

struct ScenarioPlaceWorkerTool : public EditorTool {
	explicit ScenarioPlaceWorkerTool(ScenarioDeleteWorkerTool& tool)
	   : EditorTool(tool, tool), player_(1), shipname_(""), experience_(0) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map*) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
	                         Widelands::Map*) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_gr->images().get("images/wui/editor/fsel_editor_place_worker.png");
	}
	bool has_size_one() const override {
		return true;
	}

	uint8_t get_player() const {
		return player_;
	}
	void set_player(uint8_t p) {
		player_ = p;
	}
	std::list<const Widelands::WorkerDescr*>& get_descr() {
		return descr_;
	}
	const std::string& get_shipname() const {
		return shipname_;
	}
	void set_shipname(const std::string& s) {
		shipname_ = s;
	}
	uint32_t get_experience() const {
		return experience_;
	}
	void set_experience(uint32_t e) {
		experience_ = e;
	}

private:
	uint8_t player_;
	std::list<const Widelands::WorkerDescr*> descr_;  // nullptr indicates ship
	std::string shipname_;
	uint32_t experience_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SCENARIO_WORKER_TOOL_H
