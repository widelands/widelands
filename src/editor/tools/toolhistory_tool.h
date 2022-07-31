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

#ifndef WL_EDITOR_TOOLS_TOOLHISTORY_TOOL_H
#define WL_EDITOR_TOOLS_TOOLHISTORY_TOOL_H

#include <list>

#include "editor/tools/tool.h"

struct ListItem {
	ListItem(const std::string& k, const std::string& t, ToolConf conf)
	   : key(k), title(t), data(conf), sticky(false) {
	}
	std::string key;    ///< Non-locale-dependent description
	std::string title;  ///< Locale-dependent description
	ToolConf data;
	bool sticky;
};

/// History of previously used tool settings
struct EditorHistoryTool : public EditorTool {
	EditorHistoryTool(EditorInteractive& parent) : EditorTool(parent, *this, *this) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>&,
	                          EditorActionArgs*,
	                          Widelands::Map*) override {
		return 0;
	}

	bool add_configuration(const ToolConf& conf);

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_info.png");
	}

	const ToolConf* get_configuration_for(const std::string& key);
	void remove_configuration(const std::string& key);

	WindowID get_window_id() override {
		return WindowID::ToolHistory;
	}

	void toggle_sticky(const std::string& key);
	std::list<ListItem>::iterator begin();
	std::list<ListItem>::iterator end();

private:
	std::list<ListItem>::iterator find_item(const std::string& key);
	void truncate();
	std::list<ListItem> tool_settings_;
	static constexpr int kMaxSize = 15;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_TOOLHISTORY_TOOL_H
