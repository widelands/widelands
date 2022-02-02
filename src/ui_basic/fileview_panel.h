/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_FILEVIEW_PANEL_H
#define WL_UI_BASIC_FILEVIEW_PANEL_H

#include <memory>

#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/tabpanel.h"

namespace UI {

/**
 * A panel with tabs that get filled from Lua scripts.
 */
class FileViewPanel : public TabPanel {
public:
	FileViewPanel(Panel* parent, UI::TabPanelStyle background_style);

	/// Reads the text for the active tab from 'lua_script' if it hasn't been loaded yet.
	void load_tab_contents();

	/// Adds a tab and registers its 'lua_script' for loading its text contents.
	/// 'lua_script' must return a table that contains 1 string.
	void add_tab(const std::string& title, const std::string& lua_script);

	void add_tab_without_script(const std::string& name,
	                            const std::string& title,
	                            Panel* panel,
	                            const std::string& tooltip = std::string());

private:
	void update_tab_size(size_t index);
	void layout() override;
	const int padding_;
	int contents_width_;
	int contents_height_;

	// Tab contents
	std::vector<std::unique_ptr<Box>> boxes_;
	std::vector<std::unique_ptr<MultilineTextarea>> textviews_;
	std::vector<std::string> script_paths_;
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_FILEVIEW_PANEL_H
